#ifndef MOUNTSSTORE_H
#define MOUNTSSTORE_H

#include <include/AttrCacheBfsClient.h>
#include <include/DefaultStorageClient.h>

using namespace azure::storage_lite;

class MountInfo
{
    public:
        std::string createdTime;
        std::shared_ptr<StorageBfsClientBase> client;
        MountInfo()
        {

        }
};

class MountsStore
{
    public:
        static void init(configParams configs);
        static MountsStore* get_instance();
        bool exists(const std::string path);
        bool add(const std::string path, configParams configs);
        bool remove(const std::string path);
        std::shared_ptr<StorageBfsClientBase> get(const std::string path);
        std::map<std::string, std::string> getMounts()
        {

            std::map<std::string, std::string> mounts;
            for (auto const& it: mPointCacheMap)
            {
                std::string mntPath = it.first;
                std::string createdTime = it.second.createdTime;
                if (mntPath == "/") {
                    continue;
                }
                if (mntPath.back() != '/') 
                {
                    mntPath.push_back('/');
                }
                mounts[mntPath] = createdTime;
            }

            return mounts;
        }
        configParams getDefaultConfigs()
        {
            return defaultConfigs;
        }

    private:
        MountsStore(configParams configs):
            defaultConfigs(configs)
        {

        }
        static std::shared_ptr<MountsStore> m_instance;
        std::map<std::string, MountInfo> mPointCacheMap;
        configParams defaultConfigs;
        
        MountInfo new_mount_info(configParams config_options)
        {
            
            std::shared_ptr<StorageBfsClientBase> storage_client;
            // accountName and container provided, let's use it to init client
            if (!config_options.accountName.empty() && 
                    !config_options.containerName.empty()) {
                if (config_options.useAttrCache) 
                {
                    syslog(LOG_INFO, "Initializing blobfuse with attr caching enabled");
                    storage_client = std::make_shared<AttrCacheBfsClient>(config_options);
                }
                else if (config_options.useADLS)
                {
                    syslog(LOG_INFO, "Initializing blobfuse using DataLake");
                    storage_client = std::make_shared<DataLakeBfsClient>(config_options);
                }
                else
                {
                    syslog(LOG_INFO, "Initializing blobfuse using BlockBlob");
                    storage_client = std::make_shared<BlockBlobBfsClient>(config_options);
                }
                syslog(LOG_INFO, "Testing authetication to %s ..", config_options.mntPath.c_str());
                if(storage_client->AuthenticateStorage())
                {
                    syslog(LOG_INFO, "Successfully Authenticated!");   
                }
                else
                {
                    syslog(LOG_ERR, "Authetication fail. Please check the readme for valid auth setups.");
                }     
            } else {
                    // use default client
                    syslog(LOG_INFO, "Initializing blobfuse with default storage client.");
                    storage_client = std::make_shared<DefaultStorageClient>(config_options);
                }
            
            MountInfo info = MountInfo();
            info.createdTime = config_options.createdTime;
            info.client = storage_client;
            return info;
        }
};
#endif // MOUNTSSTORE_H