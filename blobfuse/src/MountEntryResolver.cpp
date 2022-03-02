#include<MountEntryResolver.h>

MountEntryResolver* MountEntryResolver::get_instance()
{
    if(nullptr == m_instance.get())
    {
        m_instance.reset(new MountEntryResolver());
    }

    return m_instance.get();
}

std::shared_ptr<StorageBfsClientBase> MountEntryResolver::resolve(const std::string path) 
{
    std::string normalizePath = path;
    if (normalizePath.back() != '/') {
        normalizePath.push_back('/');
    }

    std::string curPrefix = normalizePath;
    size_t indexOfSlash;
    while ((indexOfSlash = curPrefix.find_last_of('/')) != std::string::npos)
    {
        curPrefix = curPrefix.substr(0, indexOfSlash);
        std::string mountPointPath = curPrefix + '/';
        if (MountsStore::get_instance()->exists(mountPointPath)) {
            return MountsStore::get_instance()->get(mountPointPath);
        }
    }
    syslog(LOG_CRIT, "No backend filesystem client matchpath %s, did you unmount the root folder ?", path.c_str());
    std::ostringstream errStream;
    errStream << "No backend filesystem client match path " << path;
    throw std::runtime_error(errStream.str());
}

std::shared_ptr<MountEntryResolver> MountEntryResolver::m_instance;