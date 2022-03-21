#include <MountsStoreRefreshManager.h>

#include <string>
#include <fstream>
#include <streambuf>
#include <iostream>

void from_json(const json &j, configParams &t)
{
	t.accountName = j.value("accountName", "");
	t.containerName = j.value("containerName", "");
    t.accountKey = j.value("accountKey", "");
    t.authType = get_auth_type(j.value("authType", "Key"));
    t.sasToken = j.value("sasToken", "");
    t.folder= j.value("folder", "");
    t.createdTime = j.value("createdTime", "");
}


void MountsStoreRefreshManager::init(
    configParams configs)
{
    if(nullptr == m_instance.get())
    {
        m_instance.reset(
            new MountsStoreRefreshManager(configs));
    }

    return;
}

MountsStoreRefreshManager* MountsStoreRefreshManager::get_instance()
{
    return m_instance.get();
}

void MountsStoreRefreshManager::StartMountsRefreshMonitor()
{
    std::thread t1(std::bind(&MountsStoreRefreshManager::RefreshMountsMonitor, this));
    t1.detach();
    syslog(LOG_INFO,"MountsStoreRefreshManager : monitor started");
}

void MountsStoreRefreshManager::RefreshMountsMonitor()
{
    try
    {
        std::string address = getRedisServerAddress();

        Redis redisClient = Redis("tcp://" + address);
        syslog(LOG_INFO,"init redis db client successfullly.");

        char* env_localtest = getenv("LOCAL_TEST_BLOBFUSE");
        bool isLocalTest = false;
        if (env_localtest) {
            isLocalTest = strcmp(env_localtest, "true") == 0 ? true : false;
        }
        syslog(LOG_INFO,"isLocalTest is set to %s", isLocalTest ? "true" : "false");
        while (true)
        {
            sleep(1);
            try
            {
                json jconfigs = json::parse("{}");

                if (isLocalTest) {
                    std::ifstream ifs("/home/gayangya/git/gaoyangxiaozhu/azure-storage-fuse/configs.json");
                    std::string mounts((std::istreambuf_iterator<char>(ifs) ), (std::istreambuf_iterator<char>()));
                    if (mounts.size() > 0) {
                        jconfigs = json::parse(mounts);
                    }
                } else {
                    OptionalString mounts = redisClient.get("mounts");
                    if (mounts) {
                        jconfigs = json::parse(*mounts);
                    }
                }
                
                std::map<std::string, configParams> fetchedMounts;

                for (auto it = jconfigs.begin(); it != jconfigs.end(); ++it) {
                    std::string mntPath = it.key();
                    if (mntPath.back() != '/')
                    {
                        mntPath.push_back('/');
                    }
                    struct configParams configs = defaultConfigs;
                    from_json(it.value(), configs);
                    std::string path = mntPath.substr(mntPath.find('/', 1)); // let's remove root path, example /synfs/mnt/ will be /mnt/
                    configs.mntPath = path;
                    fetchedMounts[path] = configs;
                }

                std::map<std::string, std::string> existedMounts = MountsStore::get_instance()->getMounts();
                RefreshMounts(existedMounts, fetchedMounts);
                
            }
            catch(const std::exception& e)
            {
                syslog(LOG_ERR, "get mount list from db failed %s", e.what());
            }
        }
    }
    catch(const std::exception& e)
    {
        syslog(LOG_ERR, "init redis db client failed with %s", e.what());
        std::thread t1(std::bind(&MountsStoreRefreshManager::destroyBlobfuseOnError, this));
        t1.detach();
    }

    syslog(LOG_ERR, "thread for polling mount list from db exit.");
}

std::shared_ptr<MountsStoreRefreshManager> MountsStoreRefreshManager::m_instance;