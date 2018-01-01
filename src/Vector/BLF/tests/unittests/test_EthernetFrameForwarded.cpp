#define BOOST_TEST_MODULE EthernetFrameForwarded
#if !defined(WIN32)
#define BOOST_TEST_DYN_LINK
#endif
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>

#include <Vector/BLF.h>

/* ETHERNET_FRAME_FORWARDED = 121 */
BOOST_AUTO_TEST_CASE(EthernetFrameForwarded)
{
    Vector::BLF::File file;
    file.open(CMAKE_CURRENT_SOURCE_DIR "/events_from_binlog/test_EthernetFrameForwarded.blf");
    BOOST_REQUIRE(file.is_open());

    Vector::BLF::ObjectHeaderBase * ohb = file.read();
    BOOST_REQUIRE(ohb != nullptr);
    BOOST_REQUIRE(ohb->objectType == Vector::BLF::ObjectType::ETHERNET_FRAME_FORWARDED);
    Vector::BLF::EthernetFrameForwarded * obj = static_cast<Vector::BLF::EthernetFrameForwarded *>(ohb);

    /* ObjectHeaderBase */
    BOOST_CHECK_EQUAL(obj->signature, Vector::BLF::ObjectSignature);
    BOOST_CHECK_EQUAL(obj->headerSize, obj->calculateHeaderSize());
    BOOST_CHECK_EQUAL(obj->headerVersion, 1);
    BOOST_CHECK_EQUAL(obj->objectSize, obj->calculateObjectSize());
    BOOST_CHECK(obj->objectType == Vector::BLF::ObjectType::ETHERNET_FRAME_FORWARDED);

    /* ObjectHeader */
    BOOST_CHECK_EQUAL(obj->objectFlags, Vector::BLF::ObjectHeader::ObjectFlags::TimeOneNans);
    BOOST_CHECK_EQUAL(obj->clientIndex, 0x1111);
    BOOST_CHECK_EQUAL(obj->objectVersion, 0);
    BOOST_CHECK_EQUAL(obj->objectTimeStamp, 0x2222222222222222);

    /* EthernetFrameForwarded */
    BOOST_CHECK_EQUAL(obj->structLength, obj->calculateStructLength());
    BOOST_CHECK_EQUAL(obj->flags, 0x1111);
    BOOST_CHECK_EQUAL(obj->channel, 0x2222);
    BOOST_CHECK_EQUAL(obj->hardwareChannel, 0x3333);
    BOOST_CHECK_EQUAL(obj->frameDuration, 0x4444444444444444);
    BOOST_CHECK_EQUAL(obj->frameChecksum, 0x55555555);
    BOOST_CHECK_EQUAL(obj->dir, 0x6666);
    BOOST_CHECK_EQUAL(obj->frameLength, 3);
    BOOST_CHECK_EQUAL(obj->frameHandle, 0x88888888);
    BOOST_CHECK_EQUAL(obj->reservedEthernetFrameForwarded, 0x99999999);
    BOOST_CHECK_EQUAL(obj->frameData[0], 0xAA);
    BOOST_CHECK_EQUAL(obj->frameData[1], 0xBB);
    BOOST_CHECK_EQUAL(obj->frameData[2], 0xCC);

    delete ohb;

    /* read next */
    ohb = file.read();
    BOOST_REQUIRE(ohb != nullptr);
    BOOST_REQUIRE(ohb->objectType == Vector::BLF::ObjectType::ETHERNET_FRAME_FORWARDED);

    delete ohb;

    /* read last */
    ohb = file.read();
    BOOST_REQUIRE(ohb != nullptr);
    BOOST_REQUIRE(ohb->objectType == Vector::BLF::ObjectType::Unknown115);

    delete ohb;

    BOOST_CHECK(file.eof());
    file.close();
}