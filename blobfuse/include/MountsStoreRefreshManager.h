#include <MountsStore.h>
#include <BlockBlobBfsClient.h>
#include <cstdint>
#include <string>
#include <json.hpp>

using json = nlohmann::json;

class MountsStoreRefreshManager {
    public:
        static void init(configParams configs, void (*destroyBlobfuseOnAuthError)());
        static MountsStoreRefreshManager* get_instance();
        void StartMountsRefreshMonitor();
        void RefreshMountsMonitor();    
    private:
        MountsStoreRefreshManager(
            configParams configs, void (*destroyBlobfuseOnAuthError)()):
            defaultConfigs(configs)
        {
            struct configParams hoboContainerConfigs = defaultConfigs;
            hoboContainerConfigs.accountName = hoboContainerConfigs.clusterAccountName;
            hoboContainerConfigs.containerName = hoboContainerConfigs.clusterContainerName;
            hoboContainerConfigs.sasToken = hoboContainerConfigs.clusterSasToken;
            hoboContainerConfigs.authType = get_auth_type("sas");
            syslog(LOG_INFO, "init hobo storage client with sas token %s", hoboContainerConfigs.sasToken.c_str());
            hobo_storage_client = std::make_shared<BlockBlobBfsClient>(hoboContainerConfigs);
            if (hobo_storage_client->AuthenticateStorage()) {
                syslog(LOG_INFO, "Successfully Authenticated for hobo storage!");   
            } else {
                syslog(LOG_ERR, "Unable to start blobfuse due to autheticated to hobo storage failed");  
                fprintf(stderr, "Unable to start blobfuse due to autheticated to hobo storage failed" );
                std::thread t1(std::bind(destroyBlobfuseOnAuthError));
                t1.detach();
            }
        }

        static std::shared_ptr<MountsStoreRefreshManager> m_instance;
        configParams defaultConfigs;
        std::shared_ptr<StorageBfsClientBase> hobo_storage_client;

        void RefreshMounts(
            std::map<std::string, std::string> existedMounts,
            std::map<std::string, configParams> fetchedMounts)
            {
                std::vector<std::string> mountPointsNeedRemoved;
                for(auto const & existedMount : existedMounts)
                {
                    std::string path = existedMount.first;
                    if (fetchedMounts.find(path) == fetchedMounts.end()) {
                        mountPointsNeedRemoved.push_back(path);
                    }
                }

                std::vector<std::pair<std::string, configParams>> mountPointsNeedAdded; 
                for(auto const & fetchedMount: fetchedMounts) 
                {
                    std::string path = fetchedMount.first;
                    configParams configs = fetchedMount.second;
                    std::string createdTime = configs.createdTime;
                    if (existedMounts.find(path) == existedMounts.end() || existedMounts[path] != createdTime) 
                    {
                        mountPointsNeedAdded.push_back(make_pair(path, configs));
                    }
                }

                // do unmount
                for (auto & path : mountPointsNeedRemoved)
                {
                    std::string mntPath(defaultConfigs.mntPath + path.substr(1));
                    syslog(LOG_INFO, "unmount %s.", mntPath.c_str());
                    MountsStore::get_instance()->remove(path);
                    syslog(LOG_INFO, "unmount %s done.", mntPath.c_str());
                }

                // do mount
                for (auto & ele : mountPointsNeedAdded)
                {
                    std::string path = ele.first;
                    configParams configs = ele.second;
                    if (path.back() != '/') {
                        path.push_back('/');
                    }
                    syslog(LOG_INFO, "mount %s.", configs.mntPath.c_str());
                    MountsStore::get_instance()->add(path, configs);
                    std::string mntPathString = prepend_mnt_path_string(path, false);
                    ensure_files_directory_exists_in_local(mntPathString.c_str());
                    mntPathString = prepend_mnt_path_string(path); // refactor
                    ensure_files_directory_exists_in_local(mntPathString.c_str());
                    syslog(LOG_INFO, "mount %s done.", configs.mntPath.c_str());
                }
            }
};

void from_json(const json &j, configParams &config);