#include "../include/filesys.h"

bool filesys_format(/*in*/ org::vfat::FileDisk *device,
                    /*in*/ uint64_t volume_size,
                    /*in*/ uint16_t bytes_per_sector,
                    /*in*/ uint16_t sectors_per_cluster,
                    /*out*/ struct filesys *fs)
{
    fs->device = device;
    fs->bootSector = new BootSector();
    fs->bootSector->Create(volume_size, bytes_per_sector, sectors_per_cluster);

    fs->fat = new Fat(fs->bootSector);
    fs->root = new ClusterChainDirectory();

    fs->bootSector->Write(device);

    fs->fat->Create();    
    fs->root->CreateRoot(fs->fat);

    fs->fat->Write(device);
    fs->root->Write(device);

    return true;
}

bool filesys_open(/*in*/ org::vfat::FileDisk *device, /*out*/ struct filesys *fs)
{
    fs->device = device;
    fs->bootSector = new BootSector();    
    fs->root = new ClusterChainDirectory();

    fs->bootSector->Read(device);
    fs->fat = new Fat(fs->bootSector);
    fs->fat->Read(device);
    fs->root->ReadRoot(device, fs->fat);

    return true;
}

bool filesys_close(/*in*/ struct filesys *fs)
{
    fs->root->Write(fs->device);
    fs->fat->Write(fs->device);
    fs->bootSector->Write(fs->device);

    return true;
}

bool filesys_destruct(/*in*/ struct filesys *fs)
{
    delete fs->root;
    delete fs->fat;
    delete fs->bootSector;

    return true;
}

static int parse_path(/*in*/ const char *path, /*out*/ char *parts[256])
{
    // Skip leading '/'
    int n = 0;
    int len;
    int i = 0;
    int j;
    while (path[i] != '\0') {
        j = i + 1;
        while(path[j] != '\0' && path[j] != '/') {
            ++j;
        }

        if (j == i + 1) {
            i = j;
            continue;
        }

        len = j - i;

        parts[n] = static_cast<char *>(malloc(sizeof(char) * len));
        memcpy(parts[n], &(path[i + 1]), len);
        parts[n][len - 1] = '\0';

        ++n;
        i = j;
    }

    return n;
}

bool filesys_mkdir(/*in*/ struct filesys *fs, /*in*/ const char *path)
{
    char *parts[256];
    int nparts = parse_path(path, parts);

    DirectoryEntry *e;
    ClusterChainDirectory *dir = fs->root;

    for (int i = 0; i < nparts; i++) {
        ClusterChainDirectory *subDir;
        e = dir->FindEntry(parts[i]);
        if (e == nullptr) {
            e = dir->AddDirectory(parts[i], fs->device);
        }

        subDir = dir->GetDirectory(fs->device, e);

        if (dir != fs->root) {
            delete dir;
        }

        dir = subDir;
    }

    if (dir != fs->root) {
        delete dir;
    }    

    for (int i = 0; i < nparts; i++) {
        free(parts[i]);
    }

    return true;
}

struct vdir * filesys_opendir(/*in*/ struct filesys *fs, /*in*/ const char *path)
{
    char *parts[256];
    int nparts = parse_path(path, parts);

    DirectoryEntry *e;
    ClusterChainDirectory *dir = fs->root;
    ClusterChainDirectory *subdir;

    bool notfound = false;
    for (int i = 0; i < nparts; ++i) {
//        if (!cchdir_findentry(dir, parts[i], &e)) {
//            notfound = true;
//            break;
//        }

        e = dir->FindEntry(parts[i]);
        if (e == nullptr) {
            notfound = true;
            break;
        }

        subdir = dir->GetDirectory(fs->device, e);

        if (dir != fs->root) {            
            delete dir;
        }

        dir = subdir;
    }

    /*if (dir != fs->root) {
        cchdir_destruct(dir);
        free(dir);
    }*/

    for (int i = 0; i < nparts; i++) {
        free(parts[i]);
    }

    if (notfound) {
        return NULL;
    }

    struct vdir *vdir = static_cast<struct vdir *>(malloc(sizeof(struct vdir)));
    vdir->ccdir = dir;
    vdir->idx = 0;

    return vdir;
}

void filesys_closedir(/*in*/ struct filesys *fs, /*in*/ struct vdir *dir)
{
    if (dir->ccdir != fs->root) {        
        delete dir->ccdir;
        free(dir->ccdir);
    }

    free(dir);
}

bool filesys_readdir(/*in*/ struct vdir *dir, /*out*/ struct vdirent *entry)
{
    uint32_t n = dir->ccdir->GetEntries()->size();
    if (dir->idx == n) {
        return false;
    }

    DirectoryEntry *e = dir->ccdir->GetEntry(dir->idx);
    e->GetName(entry->name);
    entry->isdir = e->IsDir();
    entry->datalen = e->GetDataLength();

    ++(dir->idx);

    return true;
}

struct vdir * filesys_getdir(/*in*/ struct filesys *fs, /*in*/ struct vdir *dir, /*in*/ const char *name)
{
//    DirectoryEntry e;
//    if (!cchdir_findentry(dir->ccdir, name, &e)) {
//        return NULL;
//    }

    DirectoryEntry *e = dir->ccdir->FindEntry(name);
    if (e == nullptr) {
        return nullptr;
    }

    ClusterChainDirectory *subccdir = dir->ccdir->GetDirectory(fs->device, e);
    struct vdir *subdir = static_cast<struct vdir *>(malloc(sizeof(struct vdir)));
    subdir->ccdir = subccdir;
    subdir->idx = 0;

    return subdir;
}

struct vfile * filesys_fopen(/*in*/ struct filesys *fs,
                             /*in*/ const char *fname,
                             /*in*/ const char *mode)
{
    // Parse path to file
    return NULL;
}
