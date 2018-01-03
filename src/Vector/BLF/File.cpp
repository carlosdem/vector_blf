/*
 * Copyright (C) 2013 Tobias Lorenz.
 * Contact: tobias.lorenz@gmx.net
 *
 * This file is part of Tobias Lorenz's Toolkit.
 *
 * Commercial License Usage
 * Licensees holding valid commercial licenses may use this file in
 * accordance with the commercial license agreement provided with the
 * Software or, alternatively, in accordance with the terms contained in
 * a written agreement between you and Tobias Lorenz.
 *
 * GNU General Public License 3.0 Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 3.0 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU General Public License version 3.0 requirements will be
 * met: http://www.gnu.org/copyleft/gpl.html.
 */

#include <Vector/BLF/File.h>

#include <cassert>
#include <cstring>

#include <zlib.h>

#include <Vector/BLF/Exceptions.h>

namespace Vector {
namespace BLF {

File::File() :
    openMode(),
    fileStatistics(),
    currentUncompressedFileSize(0),
    currentObjectCount(0),
    compressionLevel(6),
    defaultLogContainerSize(0x20000),
    writeUnknown115(true),
    readWriteQueue(),
    readWriteQueueMutex(),
    uncompressedFile(),
    uncompressedFileMutex(),
    compressedFile()
{
}

File::~File()
{
}

ULONGLONG File::currentFileSize()
{
    return static_cast<ULONGLONG>(compressedFile.tellp());
}

void File::open(const char * filename, std::ios_base::openmode mode)
{
    this->openMode = mode;

    if (mode & std::ios_base::in) {
        /* open file for reading */
        compressedFile.open(filename, std::ios_base::in | std::ios_base::binary);
        if (!is_open()) {
            return;
        }

        /* read file statistics */
        fileStatistics.read(compressedFile);

        /* trigger thread */
        compressedFile2UncompressedFile(this);
    }

    if (mode & std::ios_base::out) {
        /* open file for writing */
        compressedFile.open(filename, std::ios_base::out | std::ios_base::binary);
        if (!is_open()) {
            return;
        }

        /* write file statistics */
        fileStatistics.write(compressedFile);
    }

    /* fileStatistics done */
    currentUncompressedFileSize += fileStatistics.statisticsSize;
}

void File::open(const std::string & filename, std::ios_base::openmode mode)
{
    open(filename.c_str(), mode);
}

bool File::is_open() const
{
    return compressedFile.is_open();
}

bool File::eof()
{
    bool compressedFileEmpty =
        (static_cast<ULONGLONG>(compressedFile.tellg()) >= fileStatistics.fileSize) ||
        compressedFile.eof();
    bool uncompressedFileEmpty =
        (static_cast<ULONGLONG>(uncompressedFile.tellg()) >= fileStatistics.uncompressedFileSize) ||
        (uncompressedFile.tellg() >= uncompressedFile.tellp());
    return compressedFileEmpty && uncompressedFileEmpty;
}

ObjectHeaderBase * File::read()
{
    /* trigger thread */
    uncompressedFile2ReadWrite(this);

    /* read object from readWriteQueue */
    ObjectHeaderBase * ohb;
    readWriteQueueMutex.lock();
    ohb = readWriteQueue.front();
    readWriteQueue.pop_front();
    readWriteQueueMutex.unlock();

    return ohb;
}

void File::write(ObjectHeaderBase * objectHeaderBase)
{
    /* write object to readWriteQueue */
    readWriteQueueMutex.lock();
    readWriteQueue.push_back(objectHeaderBase);
    readWriteQueueMutex.unlock();

    /* trigger thread */
    readWrite2UncompressedFile(this);
}

void File::close()
{
    if (openMode & std::ios_base::out) {
        /* if data left, compress and write it */
        if (uncompressedFile.tellp() > uncompressedFile.tellg()) {
            /* trigger thread */
            uncompressedFile2CompressedFile(this);
        }

        /* write final LogContainer+Unknown115 */
        if (writeUnknown115) {
            fileStatistics.fileSizeWithoutUnknown115 = currentFileSize();

            /* write end of file message */
            Unknown115 eofMessage;
            eofMessage.write(uncompressedFile);

            /* trigger thread */
            uncompressedFile2CompressedFile(this);
        }

        /* set file statistics */
        fileStatistics.fileSize = currentFileSize();
        fileStatistics.uncompressedFileSize = currentUncompressedFileSize;
        fileStatistics.objectCount = currentObjectCount;
        //fileStatistics.objectsRead = 0;

        /* write file statistics */
        compressedFile.seekp(0);
        fileStatistics.write(compressedFile);
    }

    /* close both files */
    uncompressedFile.close();
    compressedFile.close();
}

ObjectHeaderBase * File::createObject(ObjectType type)
{
    ObjectHeaderBase * obj = nullptr;

    switch (type) {
    case ObjectType::UNKNOWN:
        break;

    case ObjectType::CAN_MESSAGE:
        obj = new CanMessage();
        break;

    case ObjectType::CAN_ERROR:
        obj = new CanErrorFrame();
        break;

    case ObjectType::CAN_OVERLOAD:
        obj = new CanOverloadFrame();
        break;

    case ObjectType::CAN_STATISTIC:
        obj = new CanDriverStatistic();
        break;

    case ObjectType::APP_TRIGGER:
        obj = new AppTrigger();
        break;

    case ObjectType::ENV_INTEGER:
    case ObjectType::ENV_DOUBLE:
    case ObjectType::ENV_STRING:
    case ObjectType::ENV_DATA:
        obj = new EnvironmentVariable();
        break;

    case ObjectType::LOG_CONTAINER:
        obj = new LogContainer();
        break;

    case ObjectType::LIN_MESSAGE:
        obj = new LinMessage();
        break;

    case ObjectType::LIN_CRC_ERROR:
        obj = new LinCrcError();
        break;

    case ObjectType::LIN_DLC_INFO:
        obj = new LinDlcInfo();
        break;

    case ObjectType::LIN_RCV_ERROR:
        obj = new LinReceiveError();
        break;

    case ObjectType::LIN_SND_ERROR:
        obj = new LinSendError();
        break;

    case ObjectType::LIN_SLV_TIMEOUT:
        obj = new LinSlaveTimeout();
        break;

    case ObjectType::LIN_SCHED_MODCH:
        obj = new LinSchedulerModeChange();
        break;

    case ObjectType::LIN_SYN_ERROR:
        obj = new LinSyncError();
        break;

    case ObjectType::LIN_BAUDRATE:
        obj = new LinBaudrateEvent();
        break;

    case ObjectType::LIN_SLEEP:
        obj = new LinSleepModeEvent();
        break;

    case ObjectType::LIN_WAKEUP:
        obj = new LinWakeupEvent();
        break;

    case ObjectType::MOST_SPY:
        obj = new MostSpy();
        break;

    case ObjectType::MOST_CTRL:
        obj = new MostCtrl();
        break;

    case ObjectType::MOST_LIGHTLOCK:
        obj = new MostLightLock();
        break;

    case ObjectType::MOST_STATISTIC:
        obj = new MostStatistic();
        break;

    case ObjectType::Reserved26:
    case ObjectType::Reserved27:
    case ObjectType::Reserved28:
        break;

    case ObjectType::FLEXRAY_DATA:
        obj = new FlexRayData();
        break;

    case ObjectType::FLEXRAY_SYNC:
        obj = new FlexRaySync();
        break;

    case ObjectType::CAN_DRIVER_ERROR:
        obj = new CanDriverError();
        break;

    case ObjectType::MOST_PKT:
        obj = new MostPkt();
        break;

    case ObjectType::MOST_PKT2:
        obj = new MostPkt2();
        break;

    case ObjectType::MOST_HWMODE:
        obj = new MostHwMode();
        break;

    case ObjectType::MOST_REG:
        obj = new MostReg();
        break;

    case ObjectType::MOST_GENREG:
        obj = new MostGenReg();
        break;

    case ObjectType::MOST_NETSTATE:
        obj = new MostNetState();
        break;

    case ObjectType::MOST_DATALOST:
        obj = new MostDataLost();
        break;

    case ObjectType::MOST_TRIGGER:
        obj = new MostTrigger();
        break;

    case ObjectType::FLEXRAY_CYCLE:
        obj = new FlexRayV6StartCycleEvent();
        break;

    case ObjectType::FLEXRAY_MESSAGE:
        obj = new FlexRayV6Message();
        break;

    case ObjectType::LIN_CHECKSUM_INFO:
        obj = new LinChecksumInfo();
        break;

    case ObjectType::LIN_SPIKE_EVENT:
        obj = new LinSpikeEvent();
        break;

    case ObjectType::CAN_DRIVER_SYNC:
        obj = new CanDriverHwSync();
        break;

    case ObjectType::FLEXRAY_STATUS:
        obj = new FlexRayStatusEvent();
        break;

    case ObjectType::GPS_EVENT:
        obj = new GpsEvent();
        break;

    case ObjectType::FR_ERROR:
        obj = new FlexRayVFrError();
        break;

    case ObjectType::FR_STATUS:
        obj = new FlexRayVFrStatus();
        break;

    case ObjectType::FR_STARTCYCLE:
        obj = new FlexRayVFrStartCycle();
        break;

    case ObjectType::FR_RCVMESSAGE:
        obj = new FlexRayVFrReceiveMsg();
        break;

    case ObjectType::REALTIMECLOCK:
        obj = new RealtimeClock();
        break;

    case ObjectType::Reserved52:
    case ObjectType::Reserved53:
        break;

    case ObjectType::LIN_STATISTIC:
        obj = new LinStatisticEvent();
        break;

    case ObjectType::J1708_MESSAGE:
    case ObjectType::J1708_VIRTUAL_MSG:
        obj = new J1708Message();
        break;

    case ObjectType::LIN_MESSAGE2:
        obj = new LinMessage2();
        break;

    case ObjectType::LIN_SND_ERROR2:
        obj = new LinSendError2();
        break;

    case ObjectType::LIN_SYN_ERROR2:
        obj = new LinSyncError2();
        break;

    case ObjectType::LIN_CRC_ERROR2:
        obj = new LinCrcError2();
        break;

    case ObjectType::LIN_RCV_ERROR2:
        obj = new LinReceiveError2();
        break;

    case ObjectType::LIN_WAKEUP2:
        obj = new LinWakeupEvent2();
        break;

    case ObjectType::LIN_SPIKE_EVENT2:
        obj = new LinSpikeEvent2();
        break;

    case ObjectType::LIN_LONG_DOM_SIG:
        obj = new LinLongDomSignalEvent();
        break;

    case ObjectType::APP_TEXT:
        obj = new AppText();
        break;

    case ObjectType::FR_RCVMESSAGE_EX:
        obj = new FlexRayVFrReceiveMsgEx();
        break;

    case ObjectType::MOST_STATISTICEX:
        obj = new MostStatisticEx();
        break;

    case ObjectType::MOST_TXLIGHT:
        obj = new MostTxLight();
        break;

    case ObjectType::MOST_ALLOCTAB:
        obj = new MostAllocTab();
        break;

    case ObjectType::MOST_STRESS:
        obj = new MostStress();
        break;

    case ObjectType::ETHERNET_FRAME:
        obj = new EthernetFrame();
        break;

    case ObjectType::SYS_VARIABLE:
        obj = new SystemVariable();
        break;

    case ObjectType::CAN_ERROR_EXT:
        obj = new CanErrorFrameExt();
        break;

    case ObjectType::CAN_DRIVER_ERROR_EXT:
        obj = new CanDriverErrorExt();
        break;

    case ObjectType::LIN_LONG_DOM_SIG2:
        obj = new LinLongDomSignalEvent2();
        break;

    case ObjectType::MOST_150_MESSAGE:
        obj = new Most150Message();
        break;

    case ObjectType::MOST_150_PKT:
        obj = new Most150Pkt();
        break;

    case ObjectType::MOST_ETHERNET_PKT:
        obj = new MostEthernetPkt();
        break;

    case ObjectType::MOST_150_MESSAGE_FRAGMENT:
        obj = new Most150MessageFragment();
        break;

    case ObjectType::MOST_150_PKT_FRAGMENT:
        obj = new Most150PktFragment();
        break;

    case ObjectType::MOST_ETHERNET_PKT_FRAGMENT:
        obj = new MostEthernetPktFragment();
        break;

    case ObjectType::MOST_SYSTEM_EVENT:
        obj = new MostSystemEvent();
        break;

    case ObjectType::MOST_150_ALLOCTAB:
        obj = new Most150AllocTab();
        break;

    case ObjectType::MOST_50_MESSAGE:
        obj = new Most50Message();
        break;

    case ObjectType::MOST_50_PKT:
        obj = new Most50Pkt();
        break;

    case ObjectType::CAN_MESSAGE2:
        obj = new CanMessage2();
        break;

    case ObjectType::LIN_UNEXPECTED_WAKEUP:
        obj = new LinUnexpectedWakeup();
        break;

    case ObjectType::LIN_SHORT_OR_SLOW_RESPONSE:
        obj = new LinShortOrSlowResponse();
        break;

    case ObjectType::LIN_DISTURBANCE_EVENT:
        obj = new LinDisturbanceEvent();
        break;

    case ObjectType::SERIAL_EVENT:
        obj = new SerialEvent();
        break;

    case ObjectType::OVERRUN_ERROR:
        obj = new DriverOverrun();
        break;

    case ObjectType::EVENT_COMMENT:
        obj = new EventComment();
        break;

    case ObjectType::WLAN_FRAME:
        obj = new WlanFrame();
        break;

    case ObjectType::WLAN_STATISTIC:
        obj = new WlanStatistic();
        break;

    case ObjectType::MOST_ECL:
        obj = new MostEcl();
        break;

    case ObjectType::GLOBAL_MARKER:
        obj = new GlobalMarker();
        break;

    case ObjectType::AFDX_FRAME:
        obj = new AfdxFrame();
        break;

    case ObjectType::AFDX_STATISTIC:
        obj = new AfdxStatistic();
        break;

    case ObjectType::KLINE_STATUSEVENT:
        obj = new KLineStatusEvent();
        break;

    case ObjectType::CAN_FD_MESSAGE:
        obj = new CanFdMessage();
        break;

    case ObjectType::CAN_FD_MESSAGE_64:
        obj = new CanFdMessage64();
        break;

    case ObjectType::ETHERNET_RX_ERROR:
        obj = new EthernetRxError();
        break;

    case ObjectType::ETHERNET_STATUS:
        obj = new EthernetStatus();
        break;

    case ObjectType::CAN_FD_ERROR_64:
        obj = new CanFdErrorFrame64();
        break;

    case ObjectType::LIN_SHORT_OR_SLOW_RESPONSE2:
        obj = new LinShortOrSlowResponse2;
        break;

    case ObjectType::AFDX_STATUS:
        obj = new AfdxStatus;
        break;

    case ObjectType::AFDX_BUS_STATISTIC:
        obj = new AfdxBusStatistic;
        break;

    case ObjectType::Reserved108:
        break;

    case ObjectType::AFDX_ERROR_EVENT:
        obj = new AfdxErrorEvent;
        break;

    case ObjectType::A429_ERROR:
        obj = new A429Error;
        break;

    case ObjectType::A429_STATUS:
        obj = new A429Status;
        break;

    case ObjectType::A429_BUS_STATISTIC:
        obj = new A429BusStatistic;
        break;

    case ObjectType::A429_MESSAGE:
        obj = new A429Message;
        break;

    case ObjectType::ETHERNET_STATISTIC:
        obj = new EthernetStatistic;
        break;

    case ObjectType::Unknown115:
        obj = new Unknown115;
        break;

    case ObjectType::Reserved116:
    case ObjectType::Reserved117:
        break;

    case ObjectType::TEST_STRUCTURE:
        obj = new TestStructure;
        break;

    case ObjectType::DIAG_REQUEST_INTERPRETATION:
        obj = new DiagRequestInterpretation;
        break;

    case ObjectType::ETHERNET_FRAME_EX:
        obj = new EthernetFrameEx;
        break;

    case ObjectType::ETHERNET_FRAME_FORWARDED:
        obj = new EthernetFrameForwarded;
        break;

    case ObjectType::ETHERNET_ERROR_EX:
        obj = new EthernetErrorEx;
        break;

    case ObjectType::ETHERNET_ERROR_FORWARDED:
        obj = new EthernetErrorForwarded;
        break;
    }

    return obj;
}

void File::compressedFile2UncompressedFile(File * file)
{
    /* condition */
    if (file->compressedFile.eof()) {
        return;
    }
    DWORD uncompressedFileSize = static_cast<DWORD>(file->uncompressedFile.tellp() - file->uncompressedFile.tellg());
    if (uncompressedFileSize >= file->defaultLogContainerSize) {
        return;
    }

    /* identify type */
    ObjectHeaderBase ohb;
    ohb.read(file->compressedFile);
    file->compressedFile.seekg(-ohb.calculateHeaderSize(), std::ios_base::cur);
    if (ohb.objectType != ObjectType::LOG_CONTAINER) {
        throw Exception("File::compressedFile2UncompressedFile(): Object read for inflation is not a log container.");
    }

    /* read LogContaier */
    LogContainer logContainer;
    logContainer.read(file->compressedFile);

    /* statistics */
    file->currentUncompressedFileSize +=
        logContainer.internalHeaderSize() +
        logContainer.uncompressedFileSize;

    if (logContainer.compressionMethod == 2) {
        /* create buffer */
        uLong bufferSize = static_cast<uLong>(logContainer.uncompressedFileSize);
        std::vector<char> buffer;
        buffer.resize(bufferSize);

        /* inflate */
        int retVal = uncompress(
             reinterpret_cast<Byte *>(buffer.data()),
             &bufferSize,
             reinterpret_cast<Byte *>(logContainer.compressedFile.data()),
             static_cast<uLong>(logContainer.compressedFileSize));
        if (retVal != Z_OK) {
            throw Exception("File::compressedFile2UncompressedFile(): uncompress error");
        }

        /* copy into uncompressedFile */
        file->uncompressedFileMutex.lock();
        file->uncompressedFile.write(buffer.data(), static_cast<std::streamsize>(bufferSize));
        file->uncompressedFileMutex.unlock();
    } else {
        file->uncompressedFileMutex.lock();
        file->uncompressedFile.write(
            reinterpret_cast<const char *>(logContainer.compressedFile.data()),
            static_cast<std::streamsize>(logContainer.compressedFileSize));
        file->uncompressedFileMutex.unlock();
    }

    /* trigger thread */
    //uncompressedFile2ReadWrite();
}

void File::uncompressedFile2CompressedFile(File * file)
{
    /* condition */
    DWORD uncompressedFileSize = static_cast<DWORD>(file->uncompressedFile.tellp() - file->uncompressedFile.tellg());
    // @todo compress only when defaultLogContainerSize reached or file is about to close */

    /* calculate size of data to compress */
    if (uncompressedFileSize > file->defaultLogContainerSize) {
        uncompressedFileSize = static_cast<DWORD>(file->defaultLogContainerSize);
    }

    /* setup new log container */
    LogContainer logContainer;
    logContainer.uncompressedFileSize = static_cast<DWORD>(uncompressedFileSize);
    if (file->compressionLevel == Z_NO_COMPRESSION) {
        logContainer.compressionMethod = 0;
        logContainer.compressedFileSize = logContainer.uncompressedFileSize;
        logContainer.compressedFile.resize(logContainer.compressedFileSize);

        /* copy data into LogContainer */
        file->uncompressedFileMutex.lock();
        file->uncompressedFile.read(
                    reinterpret_cast<char *>(logContainer.compressedFile.data()),
                    static_cast<std::streamsize>(uncompressedFileSize));
        if (file->uncompressedFile.gcount() < static_cast<std::streamsize>(uncompressedFileSize)) {
            file->uncompressedFileMutex.unlock();
            throw Exception("File::uncompressedFile2CompressedFile(): Unable to (completely) read block for compression");
        } else {
            file->uncompressedFileMutex.unlock();
        }
    } else {
        logContainer.compressionMethod = 2;

        /* create buffer */
        std::vector<char> bufferIn;
        bufferIn.resize(uncompressedFileSize);

        /* copy data into buffer */
        file->uncompressedFileMutex.lock();
        file->uncompressedFile.read(bufferIn.data(), static_cast<std::streamsize>(uncompressedFileSize));
        if (file->uncompressedFile.gcount() < static_cast<std::streamsize>(uncompressedFileSize)) {
            file->uncompressedFileMutex.unlock();
            throw Exception("File::uncompressedFile2CompressedFile(): Unable to (completely) read block for compression");
        } else {
            file->uncompressedFileMutex.unlock();
        }

        /* deflate/compress data */
        uLong bufferSizeOut = compressBound(uncompressedFileSize);
        logContainer.compressedFile.resize(bufferSizeOut); // extend
        int retVal = compress2(
            reinterpret_cast<Byte *>(logContainer.compressedFile.data()),
            &bufferSizeOut,
            reinterpret_cast<Byte *>(bufferIn.data()),
            uncompressedFileSize,
            file->compressionLevel);
        if (retVal != Z_OK) {
            throw Exception("File::uncompressedFile2CompressedFile(): compress2 error");
        }
        if (bufferSizeOut > logContainer.compressedFile.size()) {
            throw Exception("File::uncompressedFile2CompressedFile(): Compressed data exceeds buffer size");
        }
        logContainer.compressedFile.resize(bufferSizeOut); // shrink
    }

    /* write log container */
    logContainer.write(file->compressedFile);

    /* statistics */
    file->currentUncompressedFileSize +=
        logContainer.internalHeaderSize() +
        logContainer.uncompressedFileSize;

    /* drop old data */
    file->uncompressedFileMutex.lock();
    file->uncompressedFile.dropOldData(static_cast<std::streamsize>(uncompressedFileSize));
    file->uncompressedFileMutex.unlock();
}

void File::uncompressedFile2ReadWrite(File * file)
{
    /* condition */
    if (file->readWriteQueue.size() >= 10) {
        return;
    }

    ObjectHeaderBase ohb;

    /* first read some more compressed data and inflate it */
    while ((file->uncompressedFile.tellp() - file->uncompressedFile.tellg()) < ohb.calculateHeaderSize()) {
        /* trigger thread */
        compressedFile2UncompressedFile(file);
    }

    /* identify type */
    file->uncompressedFileMutex.lock();
    ohb.read(file->uncompressedFile);
    file->uncompressedFile.seekg(-ohb.calculateHeaderSize(), std::ios_base::cur);

    /* ensure sufficient data is available */
    while ((file->uncompressedFile.tellp() - file->uncompressedFile.tellg()) < ohb.objectSize) {
        /* trigger thread */
        compressedFile2UncompressedFile(file);
    }

    /* create object */
    ObjectHeaderBase * obj = createObject(ohb.objectType);
    assert(obj != nullptr);

    /* read object */
    obj->read(file->uncompressedFile);

    /* drop old data */
    file->uncompressedFile.dropOldData(static_cast<std::streamsize>(file->defaultLogContainerSize));
    file->uncompressedFileMutex.unlock();

    file->currentObjectCount++;
    file->readWriteQueueMutex.lock();
    file->readWriteQueue.push_back(obj);
    file->readWriteQueueMutex.unlock();
}

void File::readWrite2UncompressedFile(File * file)
{
    /* condition */
    if (file->readWriteQueue.empty()) {
        return;
    }

    /* get from readWriteQueue */
    ObjectHeaderBase * ohb;
    file->readWriteQueueMutex.lock();
    ohb = file->readWriteQueue.front();
    file->readWriteQueue.pop_front();
    file->readWriteQueueMutex.unlock();

    /* write into uncompressedFile */
    file->uncompressedFileMutex.lock();
    ohb->write(file->uncompressedFile);
    file->uncompressedFileMutex.unlock();

    /* if data exceeds defined logContainerSize, compress and write it into compressedFile */
    ULONGLONG logContainerSize = static_cast<ULONGLONG>(file->uncompressedFile.tellp() - file->uncompressedFile.tellg());
    if (logContainerSize >= file->defaultLogContainerSize) {
        /* trigger thread */
        uncompressedFile2CompressedFile(file);
    }

    /* statistics */
    file->currentObjectCount++;
}

}
}
