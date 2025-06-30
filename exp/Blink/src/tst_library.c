/*
Copyright (c) 2025 TrueSmartTech OÜ
All rights reserved.

This code is proprietary and confidential. Redistribution, modification, or commercial use without explicit permission is strictly prohibited.

Version 5.0.0
Author: TrueSmartTech OÜ
www.truesmarttech.com
*/

#include "tst_library.h"

static struct TSTMap *deviceIdMap = NULL;

uint16_t tstHashString(const char *String)
{
    uint16_t hash = 5381;
    int c;
    while ((c = *String++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

StructNode *searchStruct(struct TSTMap *map, const char *name)
{
    if (!map || !name)
        return NULL;
    for (uint32_t i = 0; i < map->size; i++)
    {
        struct TSTNode *node = map->table[i];
        while (node)
        {
            StructNode *snode = (StructNode *)node->data;
            if (snode && snode->name && strcmp(snode->name, name) == 0)
            {
                return snode;
            }
            node = node->next;
        }
    }
    return NULL;
}

uint8_t tstRegisterDevice(const char *device)
{
    if (!device)
        return TST_FAIL_ALLOCATE_MEMORY;

    if (!deviceIdMap)
    {
        uint8_t err = tstMapCreate(&deviceIdMap, 16);
        if (err != TST_OK)
            return err;
    }

    struct DeviceInfo *pDev = malloc(sizeof(DeviceInfo));
    if (!pDev)
        return TST_FAIL_ALLOCATE_MEMORY;

    uint8_t err = tstMapCreate(&pDev->interfaceMap, 16);
    if (err != TST_OK)
    {
        free(pDev);
        return err;
    }
    err = tstMapCreate(&pDev->structMap, 16);
    if (err != TST_OK)
    {
        tstMapDestroy(pDev->interfaceMap);
        free(pDev);
        return err;
    }

    pDev->device = strdup(device);
    if (!pDev->device)
    {
        tstMapDestroy(pDev->interfaceMap);
        tstMapDestroy(pDev->structMap);
        free(pDev);
        return TST_FAIL_ALLOCATE_MEMORY;
    }
    pDev->deviceId = tstHashString(device);

    return tstMapInsertDynamic(&deviceIdMap, pDev->deviceId, pDev, sizeof(DeviceInfo));
}

uint8_t tstRegisterInterface(const char *device, const char *interface, uint32_t maxPayloadSize)
{
    if (!device || !interface)
        return TST_FAIL_ALLOCATE_MEMORY;

    uint16_t deviceHash = tstHashString(device);
    struct DeviceInfo *dev = NULL;
    if (tstMapSearch(deviceIdMap, deviceHash, (void **)&dev) != TST_OK || !dev)
        return TST_FAIL;

    InterfaceNode *pInterface = malloc(sizeof(InterfaceNode));
    if (!pInterface)
        return TST_FAIL_ALLOCATE_MEMORY;

    pInterface->maxPayloadSize = maxPayloadSize;
    pInterface->tstMessageRxList = NULL;
    pInterface->tstMessageTxList = NULL;
    pInterface->reassemblyBuffer = NULL;
    pInterface->reassemblySize = 0;
    pInterface->expectedFragments = 0;

    pInterface->interface = strdup(interface);
    if (!pInterface->interface)
    {
        free(pInterface);
        return TST_FAIL_ALLOCATE_MEMORY;
    }
    pInterface->interfaceId = tstHashString(interface);

    return tstMapInsertDynamic(&dev->interfaceMap, pInterface->interfaceId, pInterface, sizeof(InterfaceNode));
}

uint8_t tstRegisterStruct(const char *device, const char *structName, void *pStruct, size_t sStruct)
{
    uint16_t deviceHash = tstHashString(device);
    struct DeviceInfo *dev = NULL;
    if (tstMapSearch(deviceIdMap, deviceHash, (void **)&dev) != TST_OK || !dev)
        return TST_FAIL;
    if (searchStruct(dev->structMap, structName))
        return TST_STRUCT_ALREADY_PRESENT;

    StructNode *snode = malloc(sizeof(StructNode));
    if (!snode)
        return TST_FAIL_ALLOCATE_MEMORY;
    snode->structPointer = pStruct;
    snode->structSize = sStruct;
    snode->name = strdup(structName);
    if (!snode->name)
    {
        free(snode);
        return TST_FAIL_ALLOCATE_MEMORY;
    }
    snode->structId = tstHashString(structName);

    return tstMapInsertDynamic(&dev->structMap, snode->structId, snode, sizeof(StructNode));
}

uint8_t tstInit(const TST_DeviceConfig *config)
{
    if (!config)
        return TST_FAIL_ALLOCATE_MEMORY;

    uint8_t err = tstRegisterDevice(config->name);
    if (err != TST_OK)
        return err;

    for (size_t i = 0; i < (config->nInterfaces / sizeof(TST_InterfaceConfig)); i++)
    {
        err = tstRegisterInterface(config->name, config->pInterfaces[i].interface, config->pInterfaces[i].maxSize);
        if (err != TST_OK)
            return err;
    }

    for (size_t i = 0; i < (config->nStructs / sizeof(TST_StructConfig)); i++)
    {
        err = tstRegisterStruct(config->name, config->pStructs[i].name, config->pStructs[i].pStruct, config->pStructs[i].sStruct);
        if (err != TST_OK)
            return err;
    }

    return TST_OK;
}

static uint32_t countList(struct TSTList *list)
{
    uint32_t count = 0;
    while (list)
    {
        count++;
        list = list->next;
    }
    return count;
}

uint8_t tstRx(const char *device, const char *interface, void *data, size_t size)
{
    uint16_t deviceHash = tstHashString(device);
    struct DeviceInfo *dev = NULL;
    if (tstMapSearch(deviceIdMap, deviceHash, (void **)&dev) != TST_OK || !dev)
    {
        return TST_FAIL;
    }

    uint16_t interfaceHash = tstHashString(interface);
    InterfaceNode *iface = NULL;
    if (tstMapSearch(dev->interfaceMap, interfaceHash, (void **)&iface) != TST_OK || !iface)
    {
        return TST_FAIL;
    }

    uint8_t err = 0;
    err = tstListPush(&iface->tstMessageRxList, data, size);
    err = tstEngine();
    return err;
}

uint8_t tstTx(const char *device, const char *interface, void *data, size_t *size)
{
    uint16_t deviceHash = tstHashString(device);
    struct DeviceInfo *dev = NULL;
    if (tstMapSearch(deviceIdMap, deviceHash, (void **)&dev) != TST_OK || !dev)
    {
        return TST_FAIL;
    }

    uint16_t interfaceHash = tstHashString(interface);
    InterfaceNode *iface = NULL;
    if (tstMapSearch(dev->interfaceMap, interfaceHash, (void **)&iface) != TST_OK || !iface)
    {
        return TST_FAIL;
    }

    if (iface->isOnline == 1)
    {
        uint8_t *msg = NULL;
        size_t msgSize = 0;
        if (tstListGet(&iface->tstMessageTxList, &msg, &msgSize) != TST_OK)
        {
            return TST_FAIL;
        }

        memcpy(data, msg, msgSize);
        *size = msgSize;
        free(msg);
        return TST_OK;
    }
    else
    {
        return TST_FAIL;
    }
}

uint8_t tstFindstructIdAndstructOffset(uint16_t deviceHash, uint8_t *pVar, char **varStruct, uint16_t *varAddress)
{
    struct DeviceInfo *dev = NULL;
    if (tstMapSearch(deviceIdMap, deviceHash, (void **)&dev) != TST_OK || !dev)
    {
        return TST_FAIL;
    }

    for (uint32_t i = 0; i < dev->structMap->size; i++)
    {
        struct TSTNode *node = dev->structMap->table[i];
        while (node)
        {
            StructNode *snode = (StructNode *)node->data;
            if (snode)
            {
                uintptr_t start = (uintptr_t)snode->structPointer;
                uintptr_t end = start + snode->structSize;
                if ((uintptr_t)pVar >= start && (uintptr_t)pVar < end)
                {
                    *varStruct = snode->name;
                    *varAddress = (uint16_t)((uintptr_t)pVar - start);
                    return TST_OK;
                }
            }
            node = node->next;
        }
    }
    return TST_NO_STRUCT_FOUND;
}

uint8_t *tstFindVariable(uint16_t deviceId, uint16_t structId, uint16_t structOffset)
{
    struct DeviceInfo *dev = NULL;
    if (tstMapSearch(deviceIdMap, deviceId, (void **)&dev) != TST_OK || !dev)
    {
        return NULL;
    }

    for (uint32_t i = 0; i < dev->structMap->size; i++)
    {
        struct TSTNode *node = dev->structMap->table[i];
        while (node)
        {
            StructNode *snode = (StructNode *)node->data;
            if (snode)
            {
                if (snode->structId == structId)
                {
                    if (structOffset < snode->structSize)
                    {
                        return snode->structPointer + structOffset;
                    }
                }
            }
            node = node->next;
        }
    }
    return NULL;
}

uint8_t tstVariablesGet(const char *device, const char *interface, void *pVar, size_t sVar)
{
    return tstVariablesCreateMessage(device, interface, pVar, sVar, NULL, VARIABLEGET);
}

uint8_t tstVariablesSet(const char *device, const char *interface, void *pVar, size_t sVar, void *pData)
{
    return tstVariablesCreateMessage(device, interface, pVar, sVar, pData, VARIABLESET);
}

uint8_t tstVariablesCreateMessage(const char *device, const char *interface, void *pVar, size_t sVar, void *pData, uint8_t mode)
{
    if (!device || !interface || !pVar || sVar == 0)
        return TST_FAIL_ALLOCATE_MEMORY;

    uint16_t deviceHash = tstHashString(device);
    struct DeviceInfo *dev = NULL;
    if (tstMapSearch(deviceIdMap, deviceHash, (void **)&dev) != TST_OK || !dev)
        return TST_FAIL;

    uint16_t interfaceHash = tstHashString(interface);
    InterfaceNode *iface = NULL;
    if (tstMapSearch(dev->interfaceMap, interfaceHash, (void **)&iface) != TST_OK || !iface)
        return TST_FAIL;

    // Find struct ID and offset for the variable
    char *varStruct = NULL;
    uint16_t varOffset = 0;
    if (tstFindstructIdAndstructOffset(deviceHash, pVar, &varStruct, &varOffset) != TST_OK)
        return TST_NO_STRUCT_FOUND;

    uint16_t structId = tstHashString(varStruct);

    // Calculate number of fragments needed
    size_t headerSize = (mode == VARIABLEGET) ? sizeof(tstMessageDataGet) : sizeof(tstMessageDataSet);
    size_t maxPayloadSize = iface->maxPayloadSize;
    size_t dataPerFragment = maxPayloadSize - headerSize;

    // For GET requests, we need just one fragment
    uint8_t totalFragments = 1;

    // For SET requests, we might need multiple fragments
    if (mode == VARIABLESET && sVar > 0)
    {
        totalFragments = (sVar + dataPerFragment - 1) / dataPerFragment;
        if (totalFragments == 0)
            totalFragments = 1;
    }

    // Prepare and send fragments
    for (uint8_t fragNum = 0; fragNum < totalFragments; fragNum++)
    {
        size_t offset = fragNum * dataPerFragment;
        size_t currentSize = (mode == VARIABLESET) ? ((offset + dataPerFragment) <= sVar ? dataPerFragment : (sVar - offset)) : 0;
        size_t msgSize = headerSize + currentSize;

        uint8_t *message = malloc(msgSize);
        if (!message)
            return TST_FAIL_ALLOCATE_MEMORY;

        if (mode == VARIABLEGET)
        {
            tstMessageDataGet *msg = (tstMessageDataGet *)message;
            msg->mode = VARIABLEGET;
            msg->deviceId = dev->deviceId;
            msg->total_fragments = totalFragments;
            msg->fragment_number = fragNum;
            msg->structId = structId;
            msg->structOffset = varOffset;
            msg->variableSize = sVar;
        }
        else
        {
            tstMessageDataSet *msg = (tstMessageDataSet *)message;
            msg->mode = VARIABLESET;
            msg->deviceId = dev->deviceId;
            msg->total_fragments = totalFragments;
            msg->fragment_number = fragNum;
            msg->structId = structId;
            msg->structOffset = varOffset;
            msg->variableSize = sVar;

            // Copy the data for this fragment
            memcpy(msg->data, (uint8_t *)pData + offset, currentSize);
        }

        uint8_t err = tstListPush(&iface->tstMessageTxList, message, msgSize);
        free(message);

        if (err != TST_OK)
            return err;
    }

    return TST_OK;
}

#define TST_MONITOR_HISTORY_SIZE 20  // Store last 20 monitor messages

uint8_t tstMonitorSend(const char *device, const char *interface, const char *message)
{
    if (!device || !interface || !message)
        return TST_FAIL_ALLOCATE_MEMORY;

    uint16_t deviceHash = tstHashString(device);
    struct DeviceInfo *dev = NULL;
    if (tstMapSearch(deviceIdMap, deviceHash, (void **)&dev) != TST_OK || !dev)
        return TST_FAIL;

    uint16_t interfaceHash = tstHashString(interface);
    InterfaceNode *iface = NULL;
    if (tstMapSearch(dev->interfaceMap, interfaceHash, (void **)&iface) != TST_OK || !iface)
        return TST_FAIL;

    // If offline, limit the queue size
    if (iface->isOnline == 0) {
        // Count current messages in the queue
        uint32_t queuedMsgs = countList(iface->tstMessageTxList);
        
        // If we already have our max messages, remove the oldest one
        if (queuedMsgs >= TST_MONITOR_HISTORY_SIZE) {
            uint8_t *oldMsg = NULL;
            size_t oldSize = 0;
            tstListGet(&iface->tstMessageTxList, &oldMsg, &oldSize);
            free(oldMsg); // Free the oldest message
        }
    }

    // Rest of the existing code to prepare and send the monitor message
    size_t messageLen = strlen(message);
    size_t headerSize = sizeof(tstMessageMonitorSend);
    size_t maxPayloadSize = iface->maxPayloadSize;
    size_t dataPerFragment = maxPayloadSize - headerSize;

    // Calculate number of fragments needed
    uint8_t totalFragments = (messageLen + dataPerFragment - 1) / dataPerFragment;
    if (totalFragments == 0)
        totalFragments = 1;

    // Prepare and send fragments
    for (uint8_t fragNum = 0; fragNum < totalFragments; fragNum++)
    {
        size_t offset = fragNum * dataPerFragment;
        size_t currentSize = (offset + dataPerFragment) <= messageLen ? dataPerFragment : (messageLen - offset);
        size_t msgSize = headerSize + currentSize;

        uint8_t *txMsg = malloc(msgSize);
        if (!txMsg)
            return TST_FAIL_ALLOCATE_MEMORY;

        tstMessageMonitorSend *msg = (tstMessageMonitorSend *)txMsg;
        msg->mode = MONITOR;
        msg->deviceId = dev->deviceId;
        msg->total_fragments = totalFragments;
        msg->fragment_number = fragNum;
        msg->msgLen = messageLen;

        // Copy message fragment
        memcpy(msg->message, message + offset, currentSize);

        uint8_t err = tstListPush(&iface->tstMessageTxList, txMsg, msgSize);
        free(txMsg);

        if (err != TST_OK)
            return err;
    }

    return TST_OK;
}

uint8_t tstEngine(void)
{
    if (!deviceIdMap)
        return TST_FAIL;

    for (uint32_t i = 0; i < deviceIdMap->size; i++)
    {
        struct TSTNode *dnode = deviceIdMap->table[i];
        while (dnode)
        {
            struct DeviceInfo *dev = (DeviceInfo *)dnode->data;
            if (dev)
            {
                for (uint32_t j = 0; j < dev->interfaceMap->size; j++)
                {
                    struct TSTNode *inode = dev->interfaceMap->table[j];
                    while (inode)
                    {
                        InterfaceNode *iface = (InterfaceNode *)inode->data;
                        if (iface)
                        {
                            uint8_t *rxData = NULL;
                            size_t rxSize = 0;
                            while (tstListGet(&iface->tstMessageRxList, &rxData, &rxSize) == TST_OK)
                            {
                                if (rxSize < 1)
                                {
                                    free(rxData);
                                    continue;
                                }

                                uint8_t mode = rxData[0];
                                uint8_t err = TST_OK;

                                switch (mode)
                                {
                                case VARIABLEGET:
                                {
                                    if (rxSize < sizeof(tstMessageDataGet))
                                    {
                                        free(rxData);
                                        continue;
                                    }

                                    tstMessageDataGet *msg = (tstMessageDataGet *)rxData;

                                    // Handle fragmentation
                                    if (msg->fragment_number == 0)
                                    {
                                        // First fragment, prepare reassembly if needed
                                        if (msg->total_fragments > 1)
                                        {
                                            // Clear any existing reassembly buffer
                                            if (iface->reassemblyBuffer)
                                            {
                                                free(iface->reassemblyBuffer);
                                                iface->reassemblyBuffer = NULL;
                                            }
                                        }
                                    }

                                    // Find the variable
                                    uint8_t *varPtr = tstFindVariable(msg->deviceId, msg->structId, msg->structOffset);
                                    if (varPtr && msg->variableSize > 0)
                                    {
                                        // Prepare response with variable data
                                        size_t responseSize = sizeof(tstMessageDataSet) + msg->variableSize;
                                        uint8_t *response = malloc(responseSize);
                                        if (!response)
                                        {
                                            free(rxData);
                                            continue;
                                        }

                                        tstMessageDataSet *resp = (tstMessageDataSet *)response;
                                        resp->mode = VARIABLESET;
                                        resp->deviceId = msg->deviceId;
                                        resp->total_fragments = 1; // Simple response for now
                                        resp->fragment_number = 0;
                                        resp->structId = msg->structId;
                                        resp->structOffset = msg->structOffset;
                                        resp->variableSize = msg->variableSize;

                                        // Copy the variable data
                                        memcpy(resp->data, varPtr, msg->variableSize);

                                        // Send the response
                                        err = tstListPush(&iface->tstMessageTxList, response, responseSize);
                                        free(response);
                                    }
                                    break;
                                }

                                case VARIABLESET:
                                {
                                    if (rxSize < sizeof(tstMessageDataSet))
                                    {
                                        free(rxData);
                                        continue;
                                    }

                                    tstMessageDataSet *msg = (tstMessageDataSet *)rxData;

                                    // Handle fragmentation
                                    if (msg->total_fragments > 1)
                                    {
                                        if (msg->fragment_number == 0)
                                        {
                                            // First fragment, prepare reassembly buffer
                                            if (iface->reassemblyBuffer)
                                            {
                                                free(iface->reassemblyBuffer);
                                            }

                                            size_t totalSize = sizeof(tstMessageDataSet) + msg->variableSize;
                                            iface->reassemblyBuffer = malloc(totalSize);
                                            if (!iface->reassemblyBuffer)
                                            {
                                                free(rxData);
                                                continue;
                                            }

                                            // Copy header
                                            memcpy(iface->reassemblyBuffer, msg, sizeof(tstMessageDataSet));

                                            // Calculate data size for this fragment
                                            size_t headerSize = sizeof(tstMessageDataSet);
                                            size_t dataSize = rxSize - headerSize;

                                            // Copy data portion
                                            memcpy(iface->reassemblyBuffer + headerSize, msg->data, dataSize);

                                            iface->reassemblySize = dataSize;
                                            iface->expectedFragments = msg->total_fragments;

                                            free(rxData);
                                            continue;
                                        }
                                        else if (iface->reassemblyBuffer && msg->fragment_number < iface->expectedFragments)
                                        {
                                            // Subsequent fragment
                                            size_t headerSize = sizeof(tstMessageDataSet);
                                            size_t dataSize = rxSize - headerSize;

                                            // Copy data portion to the right offset in reassembly buffer
                                            memcpy(iface->reassemblyBuffer + headerSize + iface->reassemblySize,
                                                   msg->data, dataSize);

                                            iface->reassemblySize += dataSize;

                                            // If this is the last fragment, process the reassembled message
                                            if (msg->fragment_number == iface->expectedFragments - 1)
                                            {
                                                tstMessageDataSet *fullMsg = (tstMessageDataSet *)iface->reassemblyBuffer;
                                                uint8_t *varPtr = tstFindVariable(fullMsg->deviceId, fullMsg->structId, fullMsg->structOffset);

                                                if (varPtr)
                                                {
                                                    // Update the variable with the received data
                                                    memcpy(varPtr, fullMsg->data, fullMsg->variableSize);
                                                }

                                                free(iface->reassemblyBuffer);
                                                iface->reassemblyBuffer = NULL;
                                                iface->reassemblySize = 0;
                                                iface->expectedFragments = 0;
                                            }

                                            free(rxData);
                                            continue;
                                        }
                                    }
                                    else
                                    {
                                        // Single fragment message
                                        uint8_t *varPtr = tstFindVariable(msg->deviceId, msg->structId, msg->structOffset);
                                        if (varPtr)
                                        {
                                            // Calculate data size
                                            size_t dataSize = rxSize - sizeof(tstMessageDataSet);
                                            if (dataSize > msg->variableSize)
                                            {
                                                dataSize = msg->variableSize;
                                            }

                                            // Update the variable with the received data
                                            memcpy(varPtr, msg->data, dataSize);
                                        }
                                    }
                                    break;
                                }

                                case MONITOR:
                                    break;

                                case UPDATE:
                                {
                                    if (rxSize < sizeof(tstMessageUpdateRequest))
                                    {
                                        free(rxData);
                                        continue;
                                    }

                                    tstMessageUpdateRequest *req = (tstMessageUpdateRequest *)rxData;
                                    tstProcessUpdateRequest(iface, req->deviceId, req, rxSize);
                                    break;
                                }

                                case FS:
                                {
                                    tstMessageFsRequest *req = (tstMessageFsRequest *)rxData;
                                    uint8_t err = tstFsProcessRequest(iface, req->deviceId, req, rxSize);

                                    break;
                                }

                                case ONLINE:
                                    iface->isOnline = 1;
                                    break;

                                case OFFLINE:
                                    iface->isOnline = 0;
                                    break;

                                default:
                                    break;
                                }

                                free(rxData);
                            }
                        }
                        inode = inode->next;
                    }
                }
            }
            dnode = dnode->next;
        }
    }
    return TST_OK;
}

static TstFsListFunc tstFsListHandler = NULL;
static TstFsUploadFunc tstFsUploadHandler = NULL;
static TstFsDownloadFunc tstFsDownloadHandler = NULL;
static TstFsDeleteFunc tstFsDeleteHandler = NULL;

void tstRegisterFsHandlers(TstFsListFunc listFunc, TstFsUploadFunc uploadFunc,
                           TstFsDownloadFunc downloadFunc, TstFsDeleteFunc deleteFunc)
{
    tstFsListHandler = listFunc;
    tstFsUploadHandler = uploadFunc;
    tstFsDownloadHandler = downloadFunc;
    tstFsDeleteHandler = deleteFunc;
}

// Helper function to create and send FS requests
static uint8_t tstFsCreateRequest(const char *device, const char *interface,
                                  uint8_t operation, const char *path,
                                  uint32_t offset, uint32_t size,
                                  const void *data)
{
    if (!device || !interface)
        return TST_FAIL;

    uint16_t deviceHash = tstHashString(device);
    struct DeviceInfo *dev = NULL;
    if (tstMapSearch(deviceIdMap, deviceHash, (void **)&dev) != TST_OK || !dev)
        return TST_FAIL;

    uint16_t interfaceHash = tstHashString(interface);
    InterfaceNode *iface = NULL;
    if (tstMapSearch(dev->interfaceMap, interfaceHash, (void **)&iface) != TST_OK || !iface)
        return TST_FAIL;

    // Calculate sizes
    uint16_t pathLen = path ? strlen(path) : 0;
    size_t headerSize = sizeof(tstMessageFsRequest) + pathLen;
    size_t msgSize = headerSize;

    // For write operations, add data size
    if (operation == WRITE && data && size > 0)
    {
        // For simplicity, in this minimal implementation we'll handle small writes in a single packet
        if (size + headerSize <= iface->maxPayloadSize)
        {
            msgSize += size;
        }
        else
        {
            // For large writes, we would need fragmentation which would add complexity
            // For this minimal implementation, we'll return an error
            return TST_FAIL;
        }
    }

    // Create message buffer
    uint8_t *message = malloc(msgSize);
    if (!message)
        return TST_FAIL_ALLOCATE_MEMORY;

    // Fill header
    tstMessageFsRequest *req = (tstMessageFsRequest *)message;
    req->mode = FS;
    req->deviceId = dev->deviceId;
    req->fsOperation = operation;
    req->pathLen = pathLen;
    req->offset = offset;
    req->dataSize = size;

    // Copy path
    if (path && pathLen > 0)
    {
        memcpy(req->path, path, pathLen);
    }

    // Copy data for write operations
    if (operation == WRITE && data && size > 0)
    {
        memcpy((uint8_t *)message + headerSize, data, size);
    }

    // Push to transmit queue
    uint8_t err = tstListPush(&iface->tstMessageTxList, message, msgSize);
    free(message);

    return err;
}

uint8_t tstFsList(const char *device, const char *interface, const char *path)
{
    return tstFsCreateRequest(device, interface, LIST, path, 0, 0, NULL);
}

uint8_t tstFsDownload(const char *device, const char *interface, const char *path, uint32_t offset, uint32_t size)
{
    return tstFsCreateRequest(device, interface, DOWNLOAD, path, offset, size, NULL);
}

uint8_t tstFsUpload(const char *device, const char *interface, const char *path, const void *data, uint32_t offset, uint32_t size)
{
    return tstFsCreateRequest(device, interface, UPLOAD, path, offset, size, data);
}

uint8_t tstFsDelete(const char *device, const char *interface, const char *path)
{
    return tstFsCreateRequest(device, interface, DELETE, path, 0, 0, NULL);
}

// Helper function to process FS requests and send responses
uint8_t tstFsProcessRequest(InterfaceNode *iface, uint16_t deviceId,
                                   tstMessageFsRequest *req, size_t reqSize)
{
    if (!iface || !req)
        return TST_FAIL;

    // Response data and size
    uint8_t *responseData = NULL;
    uint32_t responseSize = 0;
    uint8_t status = TST_OK;

    // Convert operation code for diagnostic message
    const char *opNames[] = {"LIST", "WRITE", "APPEND", "READ", "RENAME", "DELETE", "UPLOAD", "DOWNLOAD"};
    char opName[32] = "UNKNOWN";
    if (req->fsOperation <= 7)
    {
        strcpy(opName, opNames[req->fsOperation]);
    }

    // Create a null-terminated copy of the path
    uint16_t pathLen = req->pathLen;
    char *pathCopy = NULL;

    if (pathLen > 0)
    {
        // Allocate space for the path plus null terminator
        pathCopy = (char *)malloc(pathLen + 1);
        if (!pathCopy)
            return TST_FAIL_ALLOCATE_MEMORY;

        // Copy the path data
        memcpy(pathCopy, req->path, pathLen);

        // Add null terminator
        pathCopy[pathLen] = '\0';
    }
    else
    {
        // Empty path - use root
        pathCopy = strdup("/");
        if (!pathCopy)
            return TST_FAIL_ALLOCATE_MEMORY;
    }

    // Process the request based on operation
    switch (req->fsOperation)
    {
    case LIST:
        if (tstFsListHandler)
        {
            status = tstFsListHandler(pathCopy, &responseData, &responseSize);
        }
        else
        {
            status = TST_FAIL;
        }
        break;

    case UPLOAD:
        if (tstFsUploadHandler)
        {
            // For upload requests, data follows the request header and path
            size_t headerSize = sizeof(tstMessageFsRequest) + pathLen;

            // Ensure that we have data following the header
            if (reqSize > headerSize)
            {
                const uint8_t *data = ((const uint8_t *)req) + headerSize;
                uint32_t dataSize = reqSize - headerSize;

                // Call the upload handler with the null-terminated path
                status = tstFsUploadHandler(pathCopy, data, req->offset, dataSize);
            }
            else
            {
                status = TST_FAIL;
            }
        }
        else
        {
            status = TST_FAIL;
        }
        break;

    case DOWNLOAD:
        if (tstFsDownloadHandler)
        {
            status = tstFsDownloadHandler(pathCopy, req->offset, req->dataSize,
                                          &responseData, &responseSize);
        }
        else
        {
            status = TST_FAIL;
        }
        break;

    case DELETE:
        if (tstFsDeleteHandler)
        {
            status = tstFsDeleteHandler(pathCopy);
        }
        else
        {
            status = TST_FAIL;
        }
        break;

    default:
        status = TST_FAIL;
        break;
    }

    // Free the path copy now that we're done with it
    free(pathCopy);

    // Create and send response
    size_t headerSize = sizeof(tstMessageFsResponse);
    size_t maxPayloadSize = iface->maxPayloadSize;
    size_t dataPerFragment = maxPayloadSize - headerSize;

    // If we have no response data or it fits in one packet
    if (!responseData || responseSize <= dataPerFragment)
    {
        size_t msgSize = headerSize + (responseData ? responseSize : 0);
        uint8_t *response = malloc(msgSize);
        if (!response)
        {
            if (responseData)
                free(responseData);
            return TST_FAIL_ALLOCATE_MEMORY;
        }

        tstMessageFsResponse *resp = (tstMessageFsResponse *)response;
        resp->mode = FS;
        resp->deviceId = deviceId;
        resp->fsOperation = req->fsOperation;
        resp->status = status;
        resp->dataSize = responseSize;

        if (responseData && responseSize > 0)
        {
            memcpy(resp->data, responseData, responseSize);
        }

        uint8_t err = tstListPush(&iface->tstMessageTxList, response, msgSize);
        free(response);
        if (responseData)
            free(responseData);

        return err;
    }
    // If response data is too large, fragment it
    else
    {
        // Calculate number of fragments needed
        uint32_t totalFragments = (responseSize + dataPerFragment - 1) / dataPerFragment;

        // Process each fragment
        for (uint32_t fragNum = 0; fragNum < totalFragments; fragNum++)
        {
            uint32_t offset = fragNum * dataPerFragment;
            uint32_t currentSize = (offset + dataPerFragment <= responseSize) ? dataPerFragment : (responseSize - offset);

            // Calculate message size for this fragment
            size_t msgSize = headerSize + currentSize;
            uint8_t *response = malloc(msgSize);
            if (!response)
            {
                if (responseData)
                    free(responseData);
                return TST_FAIL_ALLOCATE_MEMORY;
            }

            // Fill header
            tstMessageFsResponse *resp = (tstMessageFsResponse *)response;
            resp->mode = FS;
            resp->deviceId = deviceId;
            resp->fsOperation = req->fsOperation;
            resp->status = status;
            resp->dataSize = currentSize;

            // Copy data for this fragment
            if (responseData && currentSize > 0)
            {
                memcpy(resp->data, responseData + offset, currentSize);
            }

            // Send fragment
            uint8_t err = tstListPush(&iface->tstMessageTxList, response, msgSize);
            free(response);

            if (err != TST_OK)
            {
                if (responseData)
                    free(responseData);
                return err;
            }
        }

        // Free the response data
        if (responseData)
            free(responseData);

        return TST_OK;
    }
}

uint8_t tstOnlineRequest(const char *device, const char *interface)
{
    if (!device || !interface)
        return TST_FAIL_ALLOCATE_MEMORY;

    uint16_t deviceHash = tstHashString(device);
    struct DeviceInfo *dev = NULL;
    if (tstMapSearch(deviceIdMap, deviceHash, (void **)&dev) != TST_OK || !dev)
        return TST_FAIL;

    uint16_t interfaceHash = tstHashString(interface);
    InterfaceNode *iface = NULL;
    if (tstMapSearch(dev->interfaceMap, interfaceHash, (void **)&iface) != TST_OK || !iface)
        return TST_FAIL;

    tstMessageOnline *msg = malloc(sizeof(tstMessageOnline));
    if (!msg)
        return TST_FAIL_ALLOCATE_MEMORY;

    msg->mode = ONLINE;
    msg->deviceId = dev->deviceId;

    uint8_t err = tstListPush(&iface->tstMessageTxList, (uint8_t *)msg, sizeof(tstMessageOnline));
    free(msg);

    return err;
}

uint8_t tstOfflineRequest(const char *device, const char *interface)
{
    if (!device || !interface)
        return TST_FAIL_ALLOCATE_MEMORY;

    uint16_t deviceHash = tstHashString(device);
    struct DeviceInfo *dev = NULL;
    if (tstMapSearch(deviceIdMap, deviceHash, (void **)&dev) != TST_OK || !dev)
        return TST_FAIL;

    uint16_t interfaceHash = tstHashString(interface);
    InterfaceNode *iface = NULL;
    if (tstMapSearch(dev->interfaceMap, interfaceHash, (void **)&iface) != TST_OK || !iface)
        return TST_FAIL;

    tstMessageOffline *msg = malloc(sizeof(tstMessageOffline));
    if (!msg)
        return TST_FAIL_ALLOCATE_MEMORY;

    msg->mode = OFFLINE;
    msg->deviceId = dev->deviceId;

    uint8_t err = tstListPush(&iface->tstMessageTxList, (uint8_t*)msg, sizeof(tstMessageOffline));
    free(msg);

    return err;
}

static TstUpdateFunc tstUpdateHandler = NULL;

void tstRegisterUpdateHandler(TstUpdateFunc updateFunc)
{
    tstUpdateHandler = updateFunc;
}

uint8_t tstProcessUpdateRequest(InterfaceNode *iface, uint16_t deviceId,
                                tstMessageUpdateRequest *req, size_t reqSize)
{
    if (!iface || !req || !tstUpdateHandler)
        return TST_FAIL;

    uint8_t status = TST_OK;

    // Calculate actual data size (remove header)
    uint32_t dataSize = req->dataSize;
    uint32_t receivedCrc = req->crc;
    uint32_t sequenceNumber = req->sequenceNumber;

    // Get pointer to data portion (follows the header)
    const uint8_t *updateData = req->data;

    // For DATA operations, validate CRC before processing
    if (req->updateOperation == DATA && dataSize > 0)
    {
        // Calculate CRC of received data
        uint32_t calculatedCrc = tstCRC32(updateData, dataSize);

        // Verify CRC matches
        if (calculatedCrc != receivedCrc)
        {
            // CRC mismatch - data is corrupted
            status = TST_FAIL;
        }
        else
        {
            // CRC is valid, process the data
            status = tstUpdateHandler(updateData, dataSize, sequenceNumber, receivedCrc, DATA);
        }
    }
    else if (req->updateOperation == START)
    {
        status = tstUpdateHandler(NULL, dataSize, 0, 0, START);
    }
    else if (req->updateOperation == END)
    {
        status = tstUpdateHandler(NULL, 0, 0, 0, END);
    }
    else
    {
        status = TST_FAIL;
    }

    return status;
}

uint8_t tstUpdateResponseSend(const char *device, const char *interface, uint8_t operation, uint8_t status, uint32_t sequenceNumber, uint32_t crc)
{
    if (!device || !interface)
        return TST_FAIL_ALLOCATE_MEMORY;

    uint16_t deviceHash = tstHashString(device);
    struct DeviceInfo *dev = NULL;
    if (tstMapSearch(deviceIdMap, deviceHash, (void **)&dev) != TST_OK || !dev)
        return TST_FAIL;

    uint16_t interfaceHash = tstHashString(interface);
    InterfaceNode *iface = NULL;
    if (tstMapSearch(dev->interfaceMap, interfaceHash, (void **)&iface) != TST_OK || !iface)
        return TST_FAIL;

    // Create response message
    size_t respSize = sizeof(tstMessageUpdateResponse);
    uint8_t *response = malloc(respSize);
    if (!response)
        return TST_FAIL_ALLOCATE_MEMORY;

    // Fill in response data
    tstMessageUpdateResponse *resp = (tstMessageUpdateResponse *)response;
    resp->mode = UPDATE;
    resp->deviceId = dev->deviceId;
    resp->updateOperation = operation;
    resp->status = status;
    resp->sequenceNumber = sequenceNumber;
    resp->crc = crc;

    // Queue message for transmission
    uint8_t err = tstListPush(&iface->tstMessageTxList, response, respSize);
    free(response);

    return err;
}

uint32_t tstCRC32(uint8_t *data, size_t size) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < size; i++)
    {
        crc ^= data[i];
        for (int j = 0; j < 8; j++)
        {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc >>= 1;
        }
    }
    return ~crc;
}

uint8_t tstListPush(struct TSTList **head_ref, uint8_t *pData, size_t sData)
{
    if (!head_ref || !pData)
    {
        return TST_FAIL_ALLOCATE_MEMORY;
    }
    struct TSTList *new_node = malloc(sizeof(struct TSTList));
    if (!new_node)
    {
        return TST_FAIL_ALLOCATE_MEMORY;
    }
    new_node->pData = malloc(sData);
    if (!new_node->pData)
    {
        free(new_node);
        return TST_FAIL_ALLOCATE_MEMORY;
    }
    memcpy(new_node->pData, pData, sData);
    new_node->sData = sData;
    new_node->next = NULL;

    if (*head_ref == NULL)
    {
        *head_ref = new_node;
    }
    else
    {
        struct TSTList *last = *head_ref;
        while (last->next != NULL)
        {
            last = last->next;
        }
        last->next = new_node;
    }
    return TST_OK;
}

uint8_t tstListGet(struct TSTList **head_ref, uint8_t **pData, size_t *sData)
{
    if (!head_ref || !(*head_ref) || !pData || !sData)
    {
        return TST_FAIL_ALLOCATE_MEMORY;
    }

    struct TSTList *temp = *head_ref;
    *pData = temp->pData;
    *sData = temp->sData;
    *head_ref = temp->next;
    free(temp);
    return TST_OK;
}

uint8_t tstMapCreate(struct TSTMap **map, uint32_t size)
{
    if (!map || size == 0)
    {
        return TST_FAIL_ALLOCATE_MEMORY;
    }
    *map = malloc(sizeof(struct TSTMap));
    if (!*map)
    {
        return TST_FAIL_ALLOCATE_MEMORY;
    }
    (*map)->size = size;
    (*map)->count = 0;
    (*map)->table = calloc(size, sizeof(struct TSTNode *));
    if (!(*map)->table)
    {
        free(*map);
        *map = NULL;
        return TST_FAIL_ALLOCATE_MEMORY;
    }
    return TST_OK;
}

static uint8_t tstMapResize(struct TSTMap **mapHandle, uint32_t newSize)
{
    if (!mapHandle || !*mapHandle)
        return TST_FAIL_ALLOCATE_MEMORY;

    struct TSTMap *oldMap = *mapHandle;
    struct TSTMap *newMap = NULL;
    uint8_t err = tstMapCreate(&newMap, newSize);
    if (err != TST_OK)
        return err;

    for (uint32_t i = 0; i < oldMap->size; i++)
    {
        struct TSTNode *node = oldMap->table[i];
        while (node)
        {
            struct TSTNode *next = node->next;
            uint32_t newIndex = node->id % newMap->size;
            node->next = newMap->table[newIndex];
            newMap->table[newIndex] = node;
            newMap->count++;
            node = next;
        }
    }
    free(oldMap->table);
    free(oldMap);
    *mapHandle = newMap;
    return TST_OK;
}

uint8_t tstMapDestroy(struct TSTMap *map)
{
    if (!map)
    {
        return TST_OK;
    }
    for (uint32_t i = 0; i < map->size; i++)
    {
        struct TSTNode *node = map->table[i];
        while (node)
        {
            struct TSTNode *temp = node;
            node = node->next;
            free(temp);
        }
    }
    free(map->table);
    free(map);
    return TST_OK;
}

uint8_t tstMapInsertDynamic(struct TSTMap **mapHandle, uint32_t id, void *data, size_t dataSize)
{
    if (!mapHandle || !*mapHandle || !data || dataSize == 0)
    {
        return TST_FAIL_ALLOCATE_MEMORY;
    }
    struct TSTMap *map = *mapHandle;
    if (map->count + 1 > (map->size * 3) / 4)
    {
        uint8_t err = tstMapResize(mapHandle, map->size * 2);
        if (err != TST_OK)
            return err;
        map = *mapHandle;
    }
    uint32_t index = id % map->size;
    struct TSTNode *new_node = malloc(sizeof(struct TSTNode));
    if (!new_node)
    {
        return TST_FAIL_ALLOCATE_MEMORY;
    }
    new_node->id = id;
    new_node->data = malloc(dataSize);
    if (!new_node->data)
    {
        free(new_node);
        return TST_FAIL_ALLOCATE_MEMORY;
    }
    memcpy(new_node->data, data, dataSize);
    new_node->sData = dataSize;
    new_node->next = map->table[index];
    map->table[index] = new_node;
    map->count++;
    return TST_OK;
}

uint8_t tstMapSearch(struct TSTMap *map, uint32_t id, void **data)
{
    if (!map || !data)
    {
        return TST_FAIL_ALLOCATE_MEMORY;
    }
    uint32_t index = id % map->size;
    struct TSTNode *node = map->table[index];
    while (node)
    {
        if (node->id == id)
        {
            *data = node->data;
            return TST_OK;
        }
        node = node->next;
    }
    *data = NULL;
    return TST_FAIL_ALLOCATE_MEMORY;
}

uint8_t tstMapDelete(struct TSTMap *map, uint32_t id)
{
    if (!map)
    {
        return TST_FAIL_ALLOCATE_MEMORY;
    }
    uint32_t index = id % map->size;
    struct TSTNode *node = map->table[index];
    struct TSTNode *prev = NULL;
    while (node)
    {
        if (node->id == id)
        {
            if (!prev)
            {
                map->table[index] = node->next;
            }
            else
            {
                prev->next = node->next;
            }
            free(node->data);
            free(node);
            return TST_OK;
        }
        prev = node;
        node = node->next;
    }
    return TST_FAIL_ALLOCATE_MEMORY;
}


// Internal timer structure
typedef struct {
    char name[64];
    uint32_t initialDuration;  // Original duration in ms
    uint32_t remainingTime;    // Remaining time in ms
    TSTTimerState state;
    TSTTimerType type;
    TSTTimerCallback callback;
    void* userData;
    bool expired;
    uint32_t timestamp;        // For debugging/logging
} TSTTimerInfo;

// Internal global state
static struct TSTMap* gTimerMap = NULL;
static struct TSTList* gExpiredTimers = NULL;
static uint32_t gActiveTimers = 0;

// Utility functions
static uint32_t calculateTimerHash(const char* name) {
    return tstCRC32((uint8_t*)name, strlen(name));
}

uint8_t tstTimerInit(void) {
    if (gTimerMap != NULL) {
        // Already initialized
        return TST_OK;
    }
    
    uint8_t result = tstMapCreate(&gTimerMap, 32);  // Start with space for 32 timers
    if (result != TST_OK) {
        return result;
    }
    
    gExpiredTimers = NULL;
    gActiveTimers = 0;
    return TST_OK;
}

uint8_t tstTimerCleanup(void) {
    uint8_t result = TST_OK;
    
    // Clean up the timer map
    if (gTimerMap != NULL) {
        result = tstMapDestroy(gTimerMap);
        gTimerMap = NULL;
    }
    
    // Clean up expired timer list
    while (gExpiredTimers != NULL) {
        uint8_t* data;
        size_t size;
        tstListGet(&gExpiredTimers, &data, &size);
        free(data);  // Free the timer data that was copied into the list
    }
    
    gActiveTimers = 0;
    return result;
}

uint8_t tstTimerLoop(uint32_t elapsedMs) {
    if (gTimerMap == NULL) {
        return TST_FAIL_NOT_INITIALIZED;
    }
    
    if (elapsedMs == 0) {
        return TST_OK;  // Nothing to do
    }
    
    // Process each timer in the map
    for (uint32_t i = 0; i < gTimerMap->size; i++) {
        struct TSTNode* node = gTimerMap->table[i];
        while (node) {
            TSTTimerInfo* timer = (TSTTimerInfo*)node->data;
            
            if (timer->state == TST_TIMER_ACTIVE) {
                // Decrement time only for active timers
                if (timer->remainingTime <= elapsedMs) {
                    // Timer has expired
                    timer->remainingTime = 0;
                    timer->expired = true;
                    timer->state = TST_TIMER_COMPLETED;
                    
                    // Add to expired queue for processing
                    uint8_t* nameCopy = malloc(strlen(timer->name) + 1);
                    if (nameCopy) {
                        strcpy((char*)nameCopy, timer->name);
                        tstListPush(&gExpiredTimers, nameCopy, strlen(timer->name) + 1);
                    }
                    
                    // If it's a periodic timer, reset it
                    if (timer->type == TST_TIMER_PERIODIC) {
                        timer->remainingTime = timer->initialDuration;
                        timer->state = TST_TIMER_ACTIVE;
                        timer->expired = false;
                    } else {
                        gActiveTimers--;
                    }
                    
                    // Execute callback if provided
                    if (timer->callback) {
                        timer->callback(timer->userData);
                    }
                } else {
                    // Decrement remaining time
                    timer->remainingTime -= elapsedMs;
                }
            }
            
            node = node->next;
        }
    }
    
    return TST_OK;
}

uint8_t tstStartTimer(const char* timerName, uint32_t durationMs) {
    return tstStartTimerWithCallback(timerName, durationMs, NULL, NULL);
}

uint8_t tstStartTimerWithCallback(const char* timerName, uint32_t durationMs, TSTTimerCallback callback, void* userData) {
    if (gTimerMap == NULL) {
        uint8_t result = tstTimerInit();
        if (result != TST_OK) {
            return result;
        }
    }
    
    if (timerName == NULL || strlen(timerName) == 0 || durationMs == 0) {
        return TST_FAIL_INVALID_PARAMETER;
    }
    
    // Check if timer already exists
    uint32_t timerId = calculateTimerHash(timerName);
    void* existingTimer;
    if (tstMapSearch(gTimerMap, timerId, &existingTimer) == TST_OK) {
        // Timer exists, update it
        TSTTimerInfo* timer = (TSTTimerInfo*)existingTimer;
        timer->initialDuration = durationMs;
        timer->remainingTime = durationMs;
        timer->state = TST_TIMER_ACTIVE;
        timer->type = TST_TIMER_ONESHOT;
        timer->callback = callback;
        timer->userData = userData;
        timer->expired = false;
        timer->timestamp = (uint32_t)time(NULL);
    } else {
        // Create a new timer
        TSTTimerInfo newTimer = {0};
        strncpy(newTimer.name, timerName, sizeof(newTimer.name) - 1);
        newTimer.initialDuration = durationMs;
        newTimer.remainingTime = durationMs;
        newTimer.state = TST_TIMER_ACTIVE;
        newTimer.type = TST_TIMER_ONESHOT;
        newTimer.callback = callback;
        newTimer.userData = userData;
        newTimer.expired = false;
        newTimer.timestamp = (uint32_t)time(NULL);
        
        uint8_t result = tstMapInsertDynamic(&gTimerMap, timerId, &newTimer, sizeof(TSTTimerInfo));
        if (result != TST_OK) {
            return result;
        }
        
        gActiveTimers++;
    }
    
    return TST_OK;
}

uint8_t tstStartPeriodicTimer(const char* timerName, uint32_t intervalMs) {
    return tstStartPeriodicTimerWithCallback(timerName, intervalMs, NULL, NULL);
}

uint8_t tstStartPeriodicTimerWithCallback(const char* timerName, uint32_t intervalMs, TSTTimerCallback callback, void* userData) {
    if (gTimerMap == NULL) {
        uint8_t result = tstTimerInit();
        if (result != TST_OK) {
            return result;
        }
    }
    
    if (timerName == NULL || strlen(timerName) == 0 || intervalMs == 0) {
        return TST_FAIL_INVALID_PARAMETER;
    }
    
    // Check if timer already exists
    uint32_t timerId = calculateTimerHash(timerName);
    void* existingTimer;
    if (tstMapSearch(gTimerMap, timerId, &existingTimer) == TST_OK) {
        // Timer exists, update it
        TSTTimerInfo* timer = (TSTTimerInfo*)existingTimer;
        timer->initialDuration = intervalMs;
        timer->remainingTime = intervalMs;
        timer->state = TST_TIMER_ACTIVE;
        timer->type = TST_TIMER_PERIODIC;
        timer->callback = callback;
        timer->userData = userData;
        timer->expired = false;
        timer->timestamp = (uint32_t)time(NULL);
    } else {
        // Create a new timer
        TSTTimerInfo newTimer = {0};
        strncpy(newTimer.name, timerName, sizeof(newTimer.name) - 1);
        newTimer.initialDuration = intervalMs;
        newTimer.remainingTime = intervalMs;
        newTimer.state = TST_TIMER_ACTIVE;
        newTimer.type = TST_TIMER_PERIODIC;
        newTimer.callback = callback;
        newTimer.userData = userData;
        newTimer.expired = false;
        newTimer.timestamp = (uint32_t)time(NULL);
        
        uint8_t result = tstMapInsertDynamic(&gTimerMap, timerId, &newTimer, sizeof(TSTTimerInfo));
        if (result != TST_OK) {
            return result;
        }
        
        gActiveTimers++;
    }
    
    return TST_OK;
}

uint8_t tstStopTimer(const char* timerName) {
    if (gTimerMap == NULL || timerName == NULL) {
        return TST_FAIL_INVALID_PARAMETER;
    }
    
    uint32_t timerId = calculateTimerHash(timerName);
    void* data;
    if (tstMapSearch(gTimerMap, timerId, &data) != TST_OK) {
        return TST_FAIL_NOT_FOUND;
    }
    
    TSTTimerInfo* timer = (TSTTimerInfo*)data;
    if (timer->state == TST_TIMER_ACTIVE || timer->state == TST_TIMER_PAUSED) {
        timer->state = TST_TIMER_CANCELLED;
        gActiveTimers--;
    }
    
    // Remove the timer from the map
    return tstMapDelete(gTimerMap, timerId);
}

uint8_t tstPauseTimer(const char* timerName) {
    if (gTimerMap == NULL || timerName == NULL) {
        return TST_FAIL_INVALID_PARAMETER;
    }
    
    uint32_t timerId = calculateTimerHash(timerName);
    void* data;
    if (tstMapSearch(gTimerMap, timerId, &data) != TST_OK) {
        return TST_FAIL_NOT_FOUND;
    }
    
    TSTTimerInfo* timer = (TSTTimerInfo*)data;
    if (timer->state == TST_TIMER_ACTIVE) {
        timer->state = TST_TIMER_PAUSED;
    }
    
    return TST_OK;
}

uint8_t tstResumeTimer(const char* timerName) {
    if (gTimerMap == NULL || timerName == NULL) {
        return TST_FAIL_INVALID_PARAMETER;
    }
    
    uint32_t timerId = calculateTimerHash(timerName);
    void* data;
    if (tstMapSearch(gTimerMap, timerId, &data) != TST_OK) {
        return TST_FAIL_NOT_FOUND;
    }
    
    TSTTimerInfo* timer = (TSTTimerInfo*)data;
    if (timer->state == TST_TIMER_PAUSED) {
        timer->state = TST_TIMER_ACTIVE;
    }
    
    return TST_OK;
}

uint8_t tstResetTimer(const char* timerName, uint32_t newDurationMs) {
    if (gTimerMap == NULL || timerName == NULL) {
        return TST_FAIL_INVALID_PARAMETER;
    }
    
    uint32_t timerId = calculateTimerHash(timerName);
    void* data;
    if (tstMapSearch(gTimerMap, timerId, &data) != TST_OK) {
        return TST_FAIL_NOT_FOUND;
    }
    
    TSTTimerInfo* timer = (TSTTimerInfo*)data;
    if (newDurationMs > 0) {
        timer->initialDuration = newDurationMs;
    }
    timer->remainingTime = timer->initialDuration;
    timer->state = TST_TIMER_ACTIVE;
    timer->expired = false;
    
    return TST_OK;
}

bool tstTimerExists(const char* timerName) {
    if (gTimerMap == NULL || timerName == NULL) {
        return false;
    }
    
    uint32_t timerId = calculateTimerHash(timerName);
    void* data;
    return (tstMapSearch(gTimerMap, timerId, &data) == TST_OK);
}

bool tstTimerExpired(const char* timerName) {
    if (gTimerMap == NULL || timerName == NULL) {
        return false;
    }
    
    uint32_t timerId = calculateTimerHash(timerName);
    void* data;
    if (tstMapSearch(gTimerMap, timerId, &data) != TST_OK) {
        return false;
    }
    
    TSTTimerInfo* timer = (TSTTimerInfo*)data;
    return timer->expired;
}

uint32_t tstGetRemainingTime(const char* timerName) {
    if (gTimerMap == NULL || timerName == NULL) {
        return 0;
    }
    
    uint32_t timerId = calculateTimerHash(timerName);
    void* data;
    if (tstMapSearch(gTimerMap, timerId, &data) != TST_OK) {
        return 0;
    }
    
    TSTTimerInfo* timer = (TSTTimerInfo*)data;
    return timer->remainingTime;
}

TSTTimerState tstGetTimerState(const char* timerName) {
    if (gTimerMap == NULL || timerName == NULL) {
        return TST_TIMER_CANCELLED;  // Default to cancelled if not found
    }
    
    uint32_t timerId = calculateTimerHash(timerName);
    void* data;
    if (tstMapSearch(gTimerMap, timerId, &data) != TST_OK) {
        return TST_TIMER_CANCELLED;
    }
    
    TSTTimerInfo* timer = (TSTTimerInfo*)data;
    return timer->state;
}

uint32_t tstGetActiveTimerCount(void) {
    return gActiveTimers;
}