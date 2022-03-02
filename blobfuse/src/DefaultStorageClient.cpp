#include <DefaultStorageClient.h>

bool DefaultStorageClient::AuthenticateStorage()
{
    return true;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"

void DefaultStorageClient::UploadFromFile(const std::string sourcePath, METADATA &metadata)
{
    return;
}

void DefaultStorageClient::UploadFromStream(std::istream &sourceStream, const std::string blobName)
{
    return;
}

void DefaultStorageClient::UploadFromStream(std::istream &sourceStream, const std::string blobName,
                                          std::vector<std::pair<std::string, std::string>> &metadata)
{
    return;
}

long int DefaultStorageClient::DownloadToFile(const std::string blobName, const std::string filePath, time_t &last_modified)
{
    return 0;
}

long int DefaultStorageClient::DownloadToStream(const std::string blobName, std::ostream &destStream,
                                              unsigned long long offset, unsigned long long size)
{
    return 0;
}

bool DefaultStorageClient::CreateDirectory(const std::string directoryPath)
{    
    return true;
}

int DefaultStorageClient::Exists(std::string pathName)
{
    return true;
}
bool DefaultStorageClient::DeleteDirectory(const std::string directoryPath)
{
    return true;
}

void DefaultStorageClient::DeleteFile(std::string path)
{
    return;
}

bool DefaultStorageClient::Copy(const std::string sourcePath, const std::string destinationPath)
{
    return true;
}

int DefaultStorageClient::List(std::string continuation, std::string prefix, const std::string delimiter, list_segmented_response &resp, int max_results)
{
    return errno;
}

bool DefaultStorageClient::IsDirectory(const char *path)
{
   return true;
}

D_RETURN_CODE DefaultStorageClient::IsDirectoryEmpty(std::string path)
{
    return D_EMPTY;
}

std::vector<std::string> DefaultStorageClient::Rename(const std::string sourcePath, const std::string destinationPath)
{
    throw std::runtime_error("no implement for default storage client");
}

std::vector<std::string> DefaultStorageClient::Rename(std::string sourcePath, std::string destinationPath, bool isDir)
{
   throw std::runtime_error("no implement for default storage client");
}

int DefaultStorageClient::ListAllItemsSegmented(
    const std::string &prefix,
    const std::string &delimiter,
    LISTALL_RES &results,
    int max_results)
{
    return errno;
}

BfsFileProperty DefaultStorageClient::GetProperties(std::string path, bool /*type_known*/) {
    throw std::runtime_error("no implement for default storage client");
}

void DefaultStorageClient::GetExtraProperties(const std::string /*pathName*/, BfsFileProperty & /*prop*/)
{
    return;
}

int DefaultStorageClient::UpdateBlobProperty(std::string /*pathStr*/, std::string /*key*/, std::string /*value*/, METADATA * /*metadata*/)
{
    return 0;
}

int DefaultStorageClient::ChangeMode(const char *, mode_t)
{
    // allow edit if it is block blob, no need to check permissions
    return 0;
}


int DefaultStorageClient::RefreshSASToken(std::string sas)
{
    return 0;
}

void DefaultStorageClient::InvalidateFile(const std::string)
{
    return;
}

void DefaultStorageClient::InvalidateDir(const std::string)
{
    return;
}
#pragma GCC diagnostic pop