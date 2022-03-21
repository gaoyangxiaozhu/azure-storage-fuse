#include <MountsStore.h>
#include <BlockBlobBfsClient.h>
#include <cstdint>
#include <mntent.h>
#include <iostream>
#include <fstream>
#include <string>
#include <json.hpp>
#include <sw/redis++/redis++.h>

using json = nlohmann::json;
using namespace sw::redis;

extern int stdErrFD;

class MountsStoreRefreshManager {
    public:
        static void init(configParams configs);
        static MountsStoreRefreshManager* get_instance();
        void StartMountsRefreshMonitor();
        void RefreshMountsMonitor();    
    private:
        MountsStoreRefreshManager(configParams configs):
            defaultConfigs(configs)
        {
        }

        static std::shared_ptr<MountsStoreRefreshManager> m_instance;
        configParams defaultConfigs;
        std::shared_ptr<StorageBfsClientBase> hobo_storage_client;

        std::string getRedisServerAddress()
        {
            std::ifstream hostfile("/etc/hosts");
            std::string line;
            std::string redisAddress;
            bool foundAddress = false;
            while (!foundAddress) {
                while (std::getline(hostfile, line))
                {
                    if (line.find("hostresolver") != std::string::npos)
                    {
                        int pos = line.find_first_of("\t");
                        redisAddress = line.substr(0, pos);
                        syslog(LOG_INFO, "get db address - %s", redisAddress.c_str());
                        foundAddress = true;
                        break;
                    }
                }

                if (!foundAddress) {
                    syslog(LOG_INFO, "can't get db address from hosts file, waitting another 5 sec..");
                    sleep(5);
                }
            }

            return redisAddress + ":6379";
        }

        bool is_directory_mounted(const char* mntDir) 
        {
            bool found = false;

            if (!defaultConfigs.basicRemountCheck) {
                syslog(LOG_INFO, "Using syscall to detect directory is already mounted or not");
                struct mntent *mnt_ent;

                FILE *mnt_list;

                mnt_list = setmntent(_PATH_MOUNTED, "r");
                while ((mnt_ent = getmntent(mnt_list))) 
                {
                    if (!strcmp(mnt_ent->mnt_dir, mntDir) && !strcmp(mnt_ent->mnt_type, "fuse")) 
                    {
                        found = true;
                        break;
                    }
                }
                endmntent(mnt_list);
            } else {
                syslog(LOG_INFO, "Reading /etc/mtab to detect directory is already mounted or not");
                ssize_t read = 0;
                size_t len = 0;
                char *line = NULL;

                FILE *fp = fopen("/etc/mtab", "r");
                if (fp != NULL) {
                    while ((read = getline(&line, &len, fp)) != -1) {
                        if (strstr(line, mntDir) != NULL && strstr(line, "fuse") != NULL) 
                        {
                            found = true;
                            break;
                        }
                    }
                    fclose(fp);
                }
            }
            
            return found;
        }

        void destroyBlobfuseOnError()
        {
            char errStr[] = "Unmounting blobfuse.\n";
            
            syslog(LOG_ERR, errStr, sizeof(errStr));
            ssize_t n = write(stdErrFD, errStr, sizeof(errStr));
            if (n == -1) {
                syslog(LOG_ERR, "Failed to report back failure.");
            }
            
            fuse_unmount(config_options.mntPath.c_str(), NULL);

            if (is_directory_mounted(config_options.mntPath.c_str())) 
            {
                char errStr[] = "Failed to unmount blobfuse. Manually unmount using fusermount command.\n";
                syslog(LOG_ERR, errStr, sizeof(errStr));
                ssize_t n = write(stdErrFD, errStr, sizeof(errStr));
                if (n == -1) {
                    syslog(LOG_ERR, "%s", errStr);
                }
            } else {
                char errStr[] = "Unmounted blobfuse successfully.\n";
                syslog(LOG_ERR, errStr, sizeof(errStr));
                ssize_t n = write(stdErrFD, errStr, sizeof(errStr));
                if (n == -1) {
                    syslog(LOG_ERR, "%s", errStr);
                }
            }
            close(stdErrFD);
            exit(1);
        }

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
                    if (path.back() != '/') {
                        path.push_back('/');
                    }
                    // TODO. remove the folder after unmount ?
                    syslog(LOG_INFO, "unmount %s.", path.c_str());
                    MountsStore::get_instance()->remove(path);
                    syslog(LOG_INFO, "unmount %s done.", path.c_str());
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