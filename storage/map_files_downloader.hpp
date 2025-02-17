#pragma once

#include "storage/downloader_queue_universal.hpp"
#include "storage/downloading_policy.hpp"
#include "storage/queued_country.hpp"

#include "platform/downloader_defines.hpp"
#include "platform/http_request.hpp"
#include "platform/safe_callback.hpp"

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace storage
{
// This interface encapsulates HTTP routines for receiving servers
// URLs and downloading a single map file.
class MapFilesDownloader
{
public:
  // Denotes bytes downloaded and total number of bytes.
  using ServersList = std::vector<std::string>;
  using ServersListCallback = platform::SafeCallback<void(ServersList const & serverList)>;

  virtual ~MapFilesDownloader() = default;

  /// Asynchronously downloads a map file, periodically invokes
  /// onProgress callback and finally invokes onDownloaded
  /// callback. Both callbacks will be invoked on the main thread.
  void DownloadMapFile(QueuedCountry && queuedCountry);

  // Removes item from m_quarantine queue when list of servers is not received.
  // Parent method must be called into override method.
  virtual void Remove(CountryId const & id);

  // Clears m_quarantine queue when list of servers is not received.
  // Parent method must be called into override method.
  virtual void Clear();

  // Returns m_quarantine queue when list of servers is not received.
  // Parent method must be called into override method.
  virtual QueueInterface const & GetQueue() const;

  static std::string MakeFullUrlLegacy(std::string const & baseUrl, std::string const & fileName, int64_t dataVersion);

  /**
   * @brief Async file download as string buffer (for small files only).
   * Request can be skipped if current servers list is empty.
   * Callback will be skipped on download error.
   * @param[in]  url        Final url part like "index.json" or "maps/210415/countries.txt".
   * @param[in]  forceReset True - force reset current request, if any.
   */
  void DownloadAsString(std::string url, std::function<bool (std::string const &)> && callback, bool forceReset = false);

  void SetServersList(ServersList const & serversList);
  void SetDownloadingPolicy(DownloadingPolicy * policy);

protected:
  bool IsDownloadingAllowed() const;
  std::vector<std::string> MakeUrlList(std::string const & relativeUrl);

  // Synchronously loads list of servers by http client.
  static ServersList LoadServersList();

private:
  /**
   * @brief This method is blocking and should be called on network thread.
   * Default implementation receives a list of all servers that can be asked
   * for a map file and invokes callback on the main thread (@see ServersListCallback as SafeCallback).
   */
  virtual void GetServersList(ServersListCallback const & callback);
  /// Asynchronously downloads the file and saves result to provided directory.
  virtual void Download(QueuedCountry && queuedCountry) = 0;

  /// @param[in]  callback  Called in main thread (@see GetServersList).
  void RunServersListAsync(std::function<void()> && callback);

  /// Current file downloading request for DownloadAsString.
  using RequestT = downloader::HttpRequest;
  std::unique_ptr<RequestT> m_fileRequest;

  ServersList m_serversList;
  /// Used as guard for m_serversList assign.
  std::atomic_bool m_isServersListRequested = false;

  DownloadingPolicy * m_downloadingPolicy = nullptr;

  // This queue accumulates download requests before
  // the servers list is received on the network thread.
  Queue m_pendingRequests;
};
}  // namespace storage
