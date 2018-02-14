/**
 * @file pa_secStore_simu.c
 *
 * Simu implementation of @ref c_pa_secStore interface
 *
 * The current implementation is quite limited, but simulates a filesystem that has a limite size
 * and can store, retreive and delete entries.
 * It is stored in RAM and therefore volatile.
 *
 * Copyright (C) Sierra Wireless Inc.
 */

#include "legato.h"
#include "interfaces.h"
#include "pa_secStore.h"
#include "simuConfig.h"

//--------------------------------------------------------------------------------------------------
/**
 * Path of the database on the filesystem.
 */
//--------------------------------------------------------------------------------------------------
#ifndef SECSTORE_RECORD_PATH
# define SECSTORE_RECORD_PATH "/legato/systems/current/config/secStore.raw"
#endif

//--------------------------------------------------------------------------------------------------
/**
 * Structure that holds the information associated with an item stored in the secure storage.
 */
//--------------------------------------------------------------------------------------------------
typedef struct __attribute__((packed)) {
    char path[SECSTOREADMIN_MAX_PATH_BYTES];
    size_t size;
    uint8_t data[LE_SECSTORE_MAX_ITEM_SIZE];
    bool isAvailable;
}
SecureStorageEntry_t;

//--------------------------------------------------------------------------------------------------
/**
 * Expected return code by PA operations.
 *
 * If LE_OK, the PA will simulate behavior of a normal secure storage.
 */
//--------------------------------------------------------------------------------------------------
static le_result_t ReturnCode = LE_OK;

//--------------------------------------------------------------------------------------------------
/**
 * Path-indexed hashmap containing links to all SFS entries.
 */
//--------------------------------------------------------------------------------------------------
static le_hashmap_Ref_t Entries = NULL;

//--------------------------------------------------------------------------------------------------
/**
 * Pool of all SFS entries.
 */
//--------------------------------------------------------------------------------------------------
static le_mem_PoolRef_t EntriesPool = NULL;

//--------------------------------------------------------------------------------------------------
/**
 * Total size of storage.
 */
//--------------------------------------------------------------------------------------------------
static const size_t TotalSize = 8192;

//--------------------------------------------------------------------------------------------------
/**
 * Flag to tell if a filesystem loading is in progress or not.
 */
//--------------------------------------------------------------------------------------------------
static bool FsLoadInProgress = false;

//--------------------------------------------------------------------------------------------------
/**
 * Set the path of an entry.
 */
//--------------------------------------------------------------------------------------------------
static void SetEntryPath
(
    SecureStorageEntry_t *entryPtr,
    const char *pathPtr
)
{
    LE_ASSERT_OK( le_utf8_Copy(entryPtr->path, pathPtr, sizeof(entryPtr->path), NULL) );
}

//--------------------------------------------------------------------------------------------------
/**
 * Delete entry.
 *
 * Actually this just marks it as not available.
 * This memory leak is on purpose as to be able to analyze the entries (deleted or not).
 */
//--------------------------------------------------------------------------------------------------
static void DeleteEntry
(
    SecureStorageEntry_t *entryPtr
)
{
    entryPtr->isAvailable = false;
}

//--------------------------------------------------------------------------------------------------
/**
 * Load entries from the file system.
 */
//--------------------------------------------------------------------------------------------------
static void LoadFileSystemEntries(void)
{
    LE_INFO("Loading secStore from " SECSTORE_RECORD_PATH);

    FsLoadInProgress = true;

    int fd = open(SECSTORE_RECORD_PATH, O_RDONLY | O_NONBLOCK);
    if (fd < 0)
    {
        LE_WARN("Unable to open " SECSTORE_RECORD_PATH);
        return;
    }

    SecureStorageEntry_t entryBuffer;
    size_t entryBufferPos = 0;

    while(true)
    {
        ssize_t readSz = 0;
        readSz = read(fd,
                      (uint8_t*)&(entryBuffer) + entryBufferPos,
                      sizeof(entryBuffer) - entryBufferPos);
        if (readSz < 0)
        {
            if (errno != EINTR)
            {
                continue;
            }

            LE_FATAL("There was an error reading " SECSTORE_RECORD_PATH ": %m");
        }
        if (readSz == 0)
        {
            LE_ASSERT(entryBufferPos == 0);
            break;
        }

        entryBufferPos += readSz;
        if (readSz < sizeof(entryBuffer))
        {
            continue;
        }

        LE_ASSERT(entryBufferPos <= sizeof(entryBuffer));
        LE_ASSERT(entryBuffer.isAvailable == true);

        LE_DEBUG("Loaded ... %s %zd", entryBuffer.path,
                                      entryBuffer.size);

        // Load the entry in memory
        pa_secStore_Write(entryBuffer.path,
                          entryBuffer.data,
                          entryBuffer.size);

        entryBufferPos = 0;
    }

    close(fd);

    FsLoadInProgress = false;
}

//--------------------------------------------------------------------------------------------------
/**
 * Store all entries on the file system.
 */
//--------------------------------------------------------------------------------------------------
static void StoreFileSystemEntries(void)
{
    if (FsLoadInProgress)
    {
        return;
    }

    int fd = open(SECSTORE_RECORD_PATH, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0)
    {
        LE_ERROR("Unable to open/create " SECSTORE_RECORD_PATH);
        return;
    }

    SecureStorageEntry_t *entryPtr = NULL;
    le_hashmap_It_Ref_t iter;
    ssize_t writeSz = 0;

    /* Iterate through entries */
    iter = le_hashmap_GetIterator(Entries);
    while (LE_OK == le_hashmap_NextNode(iter))
    {
        if (writeSz >= 0)
        {
            entryPtr = (SecureStorageEntry_t*)le_hashmap_GetValue(iter);
        }

        LE_ASSERT(entryPtr);

        if (!(entryPtr->isAvailable))
        {
            // Do not save unavailable entries
            LE_DEBUG("Discarding %s", entryPtr->path);
            continue;
        }

        LE_DEBUG("Saving %s", entryPtr->path);
        writeSz = write(fd, entryPtr, sizeof(SecureStorageEntry_t));
        if (writeSz < 0)
        {
            if (errno == EINTR)
            {
                // Retry to write the same entry on the next loop
                continue;
            }

            LE_FATAL("Unable to write " SECSTORE_RECORD_PATH ": %m");
        }

        LE_ASSERT(writeSz == sizeof(SecureStorageEntry_t));
    }

    close(fd);
}


//--------------------------------------------------------------------------------------------------
/**
 * Set the return code that should be returned by following function calls.
 */
//--------------------------------------------------------------------------------------------------
void pa_secStoreSimu_SetReturnCode
(
    le_result_t returnCode  ///< [IN] Return code expected by the system
)
{
    ReturnCode = returnCode;
}

//--------------------------------------------------------------------------------------------------
/**
 * Print the secure storage content.
 */
//--------------------------------------------------------------------------------------------------
void pa_secStoreSimu_PrintContent
(
    void
)
{
    SecureStorageEntry_t *entryPtr = NULL;
    le_hashmap_It_Ref_t iter;

    /* Iterate through entries */
    iter = le_hashmap_GetIterator(Entries);
    while (LE_OK == le_hashmap_NextNode(iter))
    {
        entryPtr = (SecureStorageEntry_t*)le_hashmap_GetValue(iter);
        LE_ASSERT(entryPtr);

        LE_INFO("%c %5zu %s", (entryPtr->isAvailable) ? '*' : ' ',
                              entryPtr->size,
                              entryPtr->path);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Writes the data in the buffer to the specified path in secure storage replacing any previously
 * written data at the same path.
 *
 * @return
 *      LE_OK if successful.
 *      LE_NO_MEMORY if there is not enough memory to store the data.
 *      LE_UNAVAILABLE if the secure storage is currently unavailable.
 *      LE_BAD_PARAMETER if the path cannot be written to because it is a directory or it would
 *                       result in an invalid path.
 *      LE_FAULT if there was some other error.
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t pa_secStore_Write
(
    const char* pathPtr,            ///< [IN] Path to write to.
    const uint8_t* bufPtr,          ///< [IN] Buffer containing the data to write.
    size_t bufSize                  ///< [IN] Size of the buffer.
)
{
    LE_INFO("Write %s %zu", pathPtr, bufSize);

    if (LE_OK != ReturnCode)
    {
        return ReturnCode;
    }

    // Determine if there is enough free space
    size_t freeSpace, totalSpace;
    if (LE_OK != pa_secStore_GetTotalSpace(&totalSpace, &freeSpace))
    {
        return LE_FAULT;
    }

    // Get the existing entry, if any
    SecureStorageEntry_t *entryPtr = le_hashmap_Get(Entries, pathPtr);
    if (NULL != entryPtr)
    {
        // Count the size of the existing element as if it was available
        freeSpace += entryPtr->size;
    }

    if ( (freeSpace < bufSize) || (LE_SECSTORE_MAX_ITEM_SIZE < bufSize) )
    {
        return LE_NO_MEMORY;
    }

    // Allocate a new element if needed
    if (NULL == entryPtr)
    {
        LE_INFO("Write new entry");
        entryPtr = (SecureStorageEntry_t*)le_mem_ForceAlloc(EntriesPool);
        memset(entryPtr, 0xFF, sizeof(SecureStorageEntry_t));
        SetEntryPath(entryPtr, pathPtr);
        le_hashmap_Put(Entries, entryPtr->path, entryPtr);
    }

    LE_INFO("Write entry %p", entryPtr);
    entryPtr->size = bufSize;
    memcpy(entryPtr->data, bufPtr, bufSize);
    entryPtr->isAvailable = true;

    // Save on disk
    StoreFileSystemEntries();

    return LE_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Reads data from the specified path in secure storage.
 *
 * @return
 *      LE_OK if successful.
 *      LE_OVERFLOW if the buffer is too small to hold all the data.  No data will be written to the
 *                  buffer in this case.
 *      LE_NOT_FOUND if the path is empty.
 *      LE_UNAVAILABLE if the secure storage is currently unavailable.
 *      LE_FAULT if there was some other error.
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t pa_secStore_Read
(
    const char* pathPtr,            ///< [IN] Path to read from.
    uint8_t* bufPtr,                ///< [OUT] Buffer to store the data in.
    size_t* bufSizePtr              ///< [IN/OUT] Size of buffer when this function is called.
                                    ///          Number of bytes read when this function returns.
)
{
    SecureStorageEntry_t *entryPtr = NULL;

    LE_INFO("Read %s %zu", pathPtr, *bufSizePtr);

    if (LE_OK != ReturnCode)
    {
        return ReturnCode;
    }

    entryPtr = le_hashmap_Get(Entries, pathPtr);
    LE_INFO("Read entry %p", entryPtr);
    if( (NULL == entryPtr) || (!entryPtr->isAvailable) )
    {
        pa_secStoreSimu_PrintContent();
        return LE_NOT_FOUND;
    }

    if (*bufSizePtr < entryPtr->size)
    {
        return LE_OVERFLOW;
    }

    *bufSizePtr = entryPtr->size;
    memcpy(bufPtr, entryPtr->data, entryPtr->size);
    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Copy the meta file to the specified path.
 *
 * @return
 *      LE_OK if successful.
 *      LE_NOT_FOUND if the meta file does not exist.
 *      LE_UNAVAILABLE if the sfs is currently unavailable.
 *      LE_FAULT if there was some other error.
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t pa_secStore_CopyMetaTo
(
    const char* pathPtr             ///< [IN] Destination path of meta file copy.
)
{
    LE_FATAL("Not handled");
    return LE_UNAVAILABLE;
}


//--------------------------------------------------------------------------------------------------
/**
 * Deletes the specified path and everything under it.
 *
 * @return
 *      LE_OK if successful.
 *      LE_NOT_FOUND if the path does not exist.
 *      LE_UNAVAILABLE if the secure storage is currently unavailable.
 *      LE_FAULT if there was an error.
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t pa_secStore_Delete
(
    const char* pathPtr             ///< [IN] Path to delete.
)
{
    SecureStorageEntry_t *entryPtr = NULL;

    LE_INFO("Delete %s", pathPtr);

    if (LE_OK != ReturnCode)
    {
        return ReturnCode;
    }

    entryPtr = le_hashmap_Get(Entries, pathPtr);
    if( (NULL == entryPtr) || (!entryPtr->isAvailable) )
    {
        return LE_NOT_FOUND;
    }

    DeleteEntry(entryPtr);

    // Save on disk
    StoreFileSystemEntries();

    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Gets the size, in bytes, of the data at the specified path and everything under it.
 *
 * @return
 *      LE_OK if successful.
 *      LE_NOT_FOUND if the path does not exist.
 *      LE_UNAVAILABLE if the secure storage is currently unavailable.
 *      LE_FAULT if there was an error.
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t pa_secStore_GetSize
(
    const char* pathPtr,            ///< [IN] Path.
    size_t* sizePtr                 ///< [OUT] Size in bytes of all items in the path.
)
{
    SecureStorageEntry_t *entryPtr = NULL;

    LE_INFO("Size %s", pathPtr);

    if (LE_OK != ReturnCode)
    {
        return ReturnCode;
    }

    entryPtr = le_hashmap_Get(Entries, pathPtr);
    if( (NULL == entryPtr) || (!entryPtr->isAvailable) )
    {
        return LE_NOT_FOUND;
    }

    *sizePtr = entryPtr->size;

    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Iterates over all entries under the specified path (non-recursive), calling the supplied callback
 * with each entry name.
 *
 * @return
 *      LE_OK if successful.
 *      LE_UNAVAILABLE if the secure storage is currently unavailable.
 *      LE_FAULT if there was some other error.
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t pa_secStore_GetEntries
(
    const char* pathPtr,                    ///< [IN] Path.
    pa_secStore_GetEntry_t getEntryFunc,    ///< [IN] Callback function to call with each entry.
    void* contextPtr                        ///< [IN] Context to be supplied to the callback.
)
{
    LE_ASSERT(NULL != getEntryFunc);
    LE_ASSERT(NULL != pathPtr);

    LE_INFO("Path %s", pathPtr);

    if (LE_OK != ReturnCode)
    {
        return ReturnCode;
    }

    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Gets the total space and the available free space in secure storage.
 *
 * @return
 *      LE_OK if successful.
 *      LE_UNAVAILABLE if the secure storage is currently unavailable.
 *      LE_FAULT if there was some other error.
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t pa_secStore_GetTotalSpace
(
    size_t* totalSpacePtr,                  ///< [OUT] Total size, in bytes, of secure storage.
    size_t* freeSizePtr                     ///< [OUT] Free space, in bytes, in secure storage.
)
{
    size_t freeSpace = TotalSize;
    SecureStorageEntry_t *entryPtr = NULL;
    le_hashmap_It_Ref_t iter;

    if (LE_OK != ReturnCode)
    {
        return ReturnCode;
    }

    /* Iterate through entries */
    iter = le_hashmap_GetIterator(Entries);
    while (LE_OK == le_hashmap_NextNode(iter))
    {
        entryPtr = (SecureStorageEntry_t*)le_hashmap_GetValue(iter);
        LE_ASSERT(entryPtr);

        if (!entryPtr->isAvailable)
        {
            continue;
        }

        LE_ASSERT(freeSpace >= entryPtr->size);
        freeSpace -= entryPtr->size;
    }

    *totalSpacePtr = TotalSize;
    *freeSizePtr = freeSpace;
    return LE_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Copies all the data from source path to destination path.  The destination path must be empty.
 *
 * @return
 *      LE_OK if successful.
 *      LE_UNAVAILABLE if the secure storage is currently unavailable.
 *      LE_FAULT if there was some other error.
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t pa_secStore_Copy
(
    const char* destPathPtr,                ///< [IN] Destination path.
    const char* srcPathPtr                  ///< [IN] Source path.
)
{
    LE_FATAL("Not handled");
    return LE_UNAVAILABLE;
}


//--------------------------------------------------------------------------------------------------
/**
 * Moves all the data from source path to destination path.  The destination path must be empty.
 *
 * @return
 *      LE_OK if successful.
 *      LE_UNAVAILABLE if the secure storage is currently unavailable.
 *      LE_FAULT if there was some other error.
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t pa_secStore_Move
(
    const char* destPathPtr,                ///< [IN] Destination path.
    const char* srcPathPtr                  ///< [IN] Source path.
)
{
    SecureStorageEntry_t *entryPtr = NULL;

    LE_INFO("Move src[%s] -> dest[%s]", srcPathPtr, destPathPtr);

    entryPtr = le_hashmap_Get(Entries, srcPathPtr);
    if ( (NULL == entryPtr) || (!entryPtr->isAvailable) )
    {
        return LE_FAULT;
    }

    if (0 == strncmp(destPathPtr, srcPathPtr, SECSTOREADMIN_MAX_PATH_BYTES))
    {
        return LE_OK;
    }

    // 'Move' entry
    SetEntryPath(entryPtr, destPathPtr);

    // Save on disk
    StoreFileSystemEntries();

    return LE_OK;
}

COMPONENT_INIT
{
    // Create a hashmap to associate entries to data
    Entries = le_hashmap_Create("secStoreEntries", 0,
                                le_hashmap_HashString,
                                le_hashmap_EqualsString);

    // Create a memory pool to store the data
    EntriesPool = le_mem_CreatePool("secStoreEntriesPool", sizeof(SecureStorageEntry_t));

    // Load from file system
    LoadFileSystemEntries();
}

