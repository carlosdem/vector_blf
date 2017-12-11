#define BOOST_TEST_MODULE MostLightLock
#if !defined(WIN32)
#define BOOST_TEST_DYN_LINK
#endif
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>

#include <Vector/BLF.h>

/* MOST_LIGHTLOCK = 24 */
BOOST_AUTO_TEST_CASE(MostLightLock)
{
    Vector::BLF::File file;
    file.open(CMAKE_CURRENT_SOURCE_DIR "/events/test_MostLightLock.blf");
    BOOST_REQUIRE(file.is_open());

    Vector::BLF::ObjectHeaderBase * ohb = file.read();
    BOOST_REQUIRE(ohb != nullptr);
    BOOST_REQUIRE(ohb->objectType == Vector::BLF::ObjectType::MOST_LIGHTLOCK);
    Vector::BLF::MostLightLock * obj = static_cast<Vector::BLF::MostLightLock *>(ohb);

    /* ObjectHeaderBase */
    BOOST_CHECK_EQUAL(obj->signature, Vector::BLF::ObjectSignature);
    BOOST_CHECK_EQUAL(obj->headerSize, obj->calculateHeaderSize());
    BOOST_CHECK_EQUAL(obj->headerVersion, 1);
    BOOST_CHECK_EQUAL(obj->objectSize, obj->calculateObjectSize());
    BOOST_CHECK(obj->objectType == Vector::BLF::ObjectType::MOST_LIGHTLOCK);

    /* ObjectHeader */
    BOOST_CHECK(obj->objectFlags == Vector::BLF::ObjectHeader::ObjectFlags::TimeOneNans);
    BOOST_CHECK_EQUAL(obj->clientIndex, 0);
    BOOST_CHECK_EQUAL(obj->objectVersion, 0);
    BOOST_CHECK_EQUAL(obj->objectTimeStamp, 8638000); // ns

    /* MostLightLock */
    BOOST_CHECK_EQUAL(obj->channel, 1);
    BOOST_CHECK_EQUAL(obj->state, 1); // Signal On + Lock
    // reserved

    delete obj;

    file.close();
}