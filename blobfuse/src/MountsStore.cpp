#include<MountsStore.h>

using namespace azure::storage_lite;

void MountsStore::init(configParams configs)
{
    if(nullptr == m_instance.get())
    {
        m_instance.reset(new MountsStore(configs));
        m_instance->add("/", configs);
    }

    return;
}
MountsStore* MountsStore::get_instance()
{
    return m_instance.get();
}

bool MountsStore::exists(const std::string path) 
{
    return mPointCacheMap.find(path) != mPointCacheMap.end();
}
        
bool MountsStore::add(const std::string path, configParams configs) 
{
    mPointCacheMap[path] = new_mount_info(configs);
    return true;
}

bool MountsStore::remove(const std::string path)
{
    if (exists(path)) 
    {
        mPointCacheMap.erase(path);
    }

    return true;
}

std::shared_ptr<StorageBfsClientBase> MountsStore::get(const std::string path)
{
    return mPointCacheMap[path].client; 
}

std::shared_ptr<MountsStore> MountsStore::m_instance;