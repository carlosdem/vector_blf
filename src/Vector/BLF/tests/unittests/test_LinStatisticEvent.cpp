#define BOOST_TEST_MODULE LinStatisticEvent
#if !defined(WIN32)
#define BOOST_TEST_DYN_LINK
#endif
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>

#include <Vector/BLF.h>

/* LIN_STATISTIC = 54 */
BOOST_AUTO_TEST_CASE(LinStatisticEvent_1)
{
    Vector::BLF::File file;
    file.open(CMAKE_CURRENT_SOURCE_DIR "/events_from_binlog/test_LinStatisticEvent.blf");
    BOOST_REQUIRE(file.is_open());

    Vector::BLF::ObjectHeaderBase * ohb = file.read();
    BOOST_REQUIRE(ohb != nullptr);
    BOOST_REQUIRE(ohb->objectType == Vector::BLF::ObjectType::LIN_STATISTIC);
    Vector::BLF::LinStatisticEvent * obj = static_cast<Vector::BLF::LinStatisticEvent *>(ohb);

    /* ObjectHeaderBase */
    BOOST_CHECK_EQUAL(obj->signature, Vector::BLF::ObjectSignature);
    BOOST_CHECK_EQUAL(obj->headerSize, obj->calculateHeaderSize());
    BOOST_CHECK_EQUAL(obj->headerVersion, 1);
    BOOST_CHECK_EQUAL(obj->objectSize, obj->calculateObjectSize());
    BOOST_CHECK(obj->objectType == Vector::BLF::ObjectType::LIN_STATISTIC);

    /* ObjectHeader */
    BOOST_CHECK_EQUAL(obj->objectFlags, Vector::BLF::ObjectHeader::ObjectFlags::TimeOneNans);
    BOOST_CHECK_EQUAL(obj->clientIndex, 0x1111);
    BOOST_CHECK_EQUAL(obj->objectVersion, 0);
    BOOST_CHECK_EQUAL(obj->objectTimeStamp, 0x2222222222222222);

    /* LinStatisticEvent */
    BOOST_CHECK_EQUAL(obj->channel, 0x1111);
    BOOST_CHECK_EQUAL(obj->reservedLinStatisticEvent1, 0);
    BOOST_CHECK_EQUAL(obj->reservedLinStatisticEvent2, 0);
    BOOST_CHECK_EQUAL(obj->busLoad, 2.0);
    BOOST_CHECK_EQUAL(obj->burstsTotal, 0x33333333);
    BOOST_CHECK_EQUAL(obj->burstsOverrun, 0x44444444);
    BOOST_CHECK_EQUAL(obj->framesSent, 0x55555555);
    BOOST_CHECK_EQUAL(obj->framesReceived, 0x66666666);
    BOOST_CHECK_EQUAL(obj->framesUnanswered, 0x77777777);
    BOOST_CHECK_EQUAL(obj->reservedLinStatisticEvent3, 0);

    delete ohb;

    /* read next */
    ohb = file.read();
    BOOST_REQUIRE(ohb != nullptr);
    BOOST_REQUIRE(ohb->objectType == Vector::BLF::ObjectType::LIN_STATISTIC);

    delete ohb;

    /* read last */
    ohb = file.read();
    BOOST_REQUIRE(ohb != nullptr);
    BOOST_REQUIRE(ohb->objectType == Vector::BLF::ObjectType::Unknown115);

    delete ohb;

    BOOST_CHECK(file.eof());
    file.close();
}

BOOST_AUTO_TEST_CASE(LinStatisticEvent_2)
{
    Vector::BLF::File file;
    file.open(CMAKE_CURRENT_SOURCE_DIR "/events_from_converter/test_LinStatisticEvent.blf");
    BOOST_REQUIRE(file.is_open());

    Vector::BLF::ObjectHeaderBase * ohb = file.read();
    BOOST_REQUIRE(ohb != nullptr);
    BOOST_REQUIRE(ohb->objectType == Vector::BLF::ObjectType::LIN_STATISTIC);
    Vector::BLF::LinStatisticEvent * obj = static_cast<Vector::BLF::LinStatisticEvent *>(ohb);

    /* ObjectHeaderBase */
    BOOST_CHECK_EQUAL(obj->signature, Vector::BLF::ObjectSignature);
    BOOST_CHECK_EQUAL(obj->headerSize, obj->calculateHeaderSize());
    BOOST_CHECK_EQUAL(obj->headerVersion, 1);
    BOOST_CHECK_EQUAL(obj->objectSize, obj->calculateObjectSize());
    BOOST_CHECK(obj->objectType == Vector::BLF::ObjectType::LIN_STATISTIC);

    /* ObjectHeader */
    BOOST_CHECK_EQUAL(obj->objectFlags, Vector::BLF::ObjectHeader::ObjectFlags::TimeOneNans);
    // reserved
    BOOST_CHECK_EQUAL(obj->objectVersion, 0);
    BOOST_CHECK_EQUAL(obj->objectTimeStamp, 1999580000); // ns

    /* LinStatisticEvent */
    BOOST_CHECK_EQUAL(obj->channel, 1);
    BOOST_CHECK_EQUAL(obj->busLoad, 0.903601);
    BOOST_CHECK_EQUAL(obj->burstsOverrun, 0);
    BOOST_CHECK_EQUAL(obj->framesSent, 0);
    BOOST_CHECK_EQUAL(obj->framesReceived, 73);
    BOOST_CHECK_EQUAL(obj->framesUnanswered, 0);

    delete ohb;

    BOOST_CHECK(file.eof());
    file.close();
}
