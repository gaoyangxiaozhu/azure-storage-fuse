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
    configParams configs, void (*destroyBlobfuseOnAuthError)())
{
    if(nullptr == m_instance.get())
    {
        m_instance.reset(
            new MountsStoreRefreshManager(configs, destroyBlobfuseOnAuthError));
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
    syslog(LOG_WARNING,"MountsStoreRefreshManager : monitor started");
}

void MountsStoreRefreshManager::RefreshMountsMonitor()
{
    while (true)
    {
        sleep(1);
        time_t last_modified = {};
        std::string tmp_DiskPath = "/tmp/mount.json";
        try
        {
            hobo_storage_client->DownloadToFile("mount.json", tmp_DiskPath, last_modified);
        }
        catch(const std::exception& e)
        {
            syslog(LOG_ERR, "fetch mount list from hobo container failed %s.", e.what());
        }

        std::ifstream stream(tmp_DiskPath);
        std::string mounts_configs_str((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
        json jconfigs = json::parse(mounts_configs_str);

        std::map<std::string, configParams> fetchedMounts;

        for (auto it = jconfigs.begin(); it != jconfigs.end(); ++it) {
            std::string mntPath = it.key();
            if (mntPath.back() != '/')
            {
                mntPath.push_back('/');
            }
            struct configParams configs = defaultConfigs;
            from_json(it.value(), configs);
            std::string path = mntPath.substr(mntPath.find('/', 1)); // let's remove root path, example /tridenfs/mnt/ will be /mnt/
            configs.mntPath = path;
            fetchedMounts[path] = configs;
        }

        std::map<std::string, std::string> existedMounts = MountsStore::get_instance()->getMounts();
        RefreshMounts(existedMounts, fetchedMounts);
        unlink(tmp_DiskPath.c_str());
    }
}

std::shared_ptr<MountsStoreRefreshManager> MountsStoreRefreshManager::m_instance;