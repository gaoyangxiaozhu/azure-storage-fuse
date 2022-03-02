#ifndef MOUNTENTRYRESOLVER_H
#define MOUNTENTRYRESOLVER_H

#include <MountsStore.h>

using namespace azure::storage_lite;

class MountEntryResolver
{
    public:    
        static MountEntryResolver* get_instance();
        std::shared_ptr<StorageBfsClientBase> resolve(const std::string path);
    private:
        MountEntryResolver()
        {
            
        }
        static std::shared_ptr<MountEntryResolver> m_instance;
};

#endif // MOUNTENTRYRESOLVER_H