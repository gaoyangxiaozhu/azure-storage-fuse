#ifndef DEFAULTSTORAGECLIENT_H
#define DEFAULTSTORAGECLIENT_H

#include <StorageBfsClientBase.h>

using namespace azure::storage_lite;

class DefaultStorageClient: public StorageBfsClientBase
{
public:
    DefaultStorageClient(configParams opt):
    StorageBfsClientBase(opt)
    {}

    bool isDefault() { return true; }
    bool isADLS() { return false; }
    ///<summary>
    /// Authenticates the storage account and container
    ///</summary>
    ///<returns>bool: if we authenticate to the storage account and container successfully</returns>
    bool AuthenticateStorage() override;
    ///<summary>
    /// Uploads contents of a file to a block blob to the Storage service
    ///</summary>
    ///TODO: params
    ///<returns>none</returns>
    void UploadFromFile(const std::string sourcePath, METADATA &metadata) override;
    ///<summary>
    /// Uploads contents of a stream to a block blob to the Storage service
    ///</summary>
    ///<returns>none</returns>
    void UploadFromStream(std::istream & sourceStream, const std::string blobName) override;
    void UploadFromStream(std::istream & sourceStream, const std::string blobName, 
                std::vector<std::pair<std::string, std::string>> & metadata) override;

    ///<summary>
    /// Downloads contents of a block blob to a local file
    ///</summary>
    ///<returns>none</returns>
    long int DownloadToFile(const std::string blobName, const std::string filePath, time_t& last_modified) override;
    long int DownloadToStream(const std::string blobName, std::ostream & destStream,
                unsigned long long offset, unsigned long long size) override;
    ///<summary>
    /// Creates a Directory
    ///</summary>
    ///<returns>none</returns>
    bool CreateDirectory(const std::string directoryPath) override;
    ///<summary>
    /// Deletes a Directory
    ///</summary>
    ///<returns>none</returns>
    bool DeleteDirectory(const std::string directoryPath) override;
    ///<summary>
    /// Checks if the blob is a directory
    ///</summary>
    ///<returns>none</returns>
    bool IsDirectory(const char * path) override;
    ///<summary>
    /// Helper function - Checks if the "directory" blob is empty
    ///</summary>
    D_RETURN_CODE IsDirectoryEmpty(std::string path) override;
    ///<summary>
    /// Deletes a File
    ///</summary>
    ///<returns>none</returns>
    void DeleteFile(std::string pathToDelete) override;
    ///<summary>
    /// Gets the properties of a path
    ///</summary>
    ///<returns>BfsFileProperty object which contains the property details of the file</returns>
    BfsFileProperty GetProperties(std::string pathName, bool type_known = false) override;
    void GetExtraProperties(const std::string pathName, BfsFileProperty &prop) override;

    ///<summary>
    /// Determines whether or not a path (file or directory) exists or not
    ///</summary>
    ///<returns>none</returns>
    int Exists(std::string pathName) override;
    ///<summary>
    /// Determines whether or not a path (file or directory) exists or not
    ///</summary>
    ///<returns>none</returns>
    bool Copy(std::string sourcePath, std::string destinationPath) override;
    ///<summary>
    /// Renames a file
    ///</summary>
    ///<returns>none</returns>
    std::vector<std::string> Rename(std::string sourcePath, std::string destinationPath) override;
    std::vector<std::string> Rename(const std::string sourcePath,const  std::string destinationPath, bool isDir) override;
    ///<summary>
    /// Lists
    ///</summary>
    ///<returns>none</returns>
    int List(std::string continuation, std::string prefix, std::string delimiter, list_segmented_response &resp, int max_results = MAX_GET_LIST_RESULT_LIMIT) override;
    ///<summary>
    /// LIsts all directories within a list container
    /// Greedily list all blobs using the input params.
    ///</summary>
    int ListAllItemsSegmented(const std::string& prefix, const std::string& delimiter, LISTALL_RES &list_results, int max_results = MAX_GET_LIST_RESULT_LIMIT) override;
    ///<summary>
    /// Updates the UNIX-style file mode on a path.
    ///</summary>
    int ChangeMode(const char* path, mode_t mode) override;

    int UpdateBlobProperty(std::string pathStr, std::string key, std::string value, METADATA *metadata = NULL);
    int RefreshSASToken(std::string sas);
    void InvalidateFile(const std::string blob);
    void InvalidateDir(const std::string dir);
};
#endif // DEFAULTSTORAGECLIENT_H