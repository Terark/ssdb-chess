
#include "gtest/gtest.h"

#include "../include.h"
#include "const.h"
#include "ssdb.h"
#include "ssdb_impl.h"

int Factorial(int n) {
    if (n == 1 || n == 2) return 1;
    return Factorial(n - 1) * Factorial(n - 2);
}

using std::cout;
using std::endl;

static const std::string kDBPath = "./testdb";

// The fixture for testing class Foo.
class THashTest : public ::testing::Test {
protected:
    virtual void SetUp() {
	// Code here will be called immediately after the constructor (right
	// before each test).
	Options options;

	// open DB
	_ssdb = SSDB::open(Options(), kDBPath);
	//rocksdb::Status s = rocksdb::DB::Open(options, kDBPath, &_db);
	//assert(s.ok());
    }

    virtual void TearDown() {
	// Code here will be called immediately after each test (right
	// before the destructor).
	delete _ssdb;

	std::string cmd = "rm -rf " + kDBPath + "/*";
	system (cmd.c_str());
    }

    // Objects declared here can be used by all tests in the test case for ....
protected:
    rocksdb::DB* _db;
    SSDB *_ssdb;
};


TEST_F(THashTest, SetAndCnt) {

    std::string key1 = "key1", field1 = "field1";
    int cnt = _ssdb->hsize(key1);
    ASSERT_EQ(0, cnt);
	
    int ret = _ssdb->hset(key1, field1, "value1", BinlogCommand::HSET);
    ASSERT_NE(-1, ret);
    cnt = _ssdb->hsize(key1);
    ASSERT_EQ(1, cnt);

    std::string field2 = "field2";
    ret = _ssdb->hset(key1, field2, "value2", BinlogCommand::HSET);
    cnt = _ssdb->hsize(key1);
    ASSERT_EQ(2, cnt);

    // update field1 & field2
    ret = _ssdb->hset(key1, field1, "value1-update", BinlogCommand::HSET);
    cnt = _ssdb->hsize(key1);
    ASSERT_EQ(2, cnt);

    ret = _ssdb->hset(key1, field2, "value2-update", BinlogCommand::HSET);
    cnt = _ssdb->hsize(key1);
    ASSERT_EQ(2, cnt);
}

TEST_F(THashTest, DelAndCnt) {

    std::string key1 = "key1", field1 = "field1", field2 = "field2";
    int ret = _ssdb->hset(key1, field1, "value1", BinlogCommand::HSET);
    int cnt = _ssdb->hsize(key1);
    ASSERT_EQ(1, cnt);

    ret = _ssdb->hdel(key1, field1, BinlogCommand::HDEL);
    ASSERT_EQ(1, ret);
    cnt = _ssdb->hsize(key1);
    ASSERT_EQ(0, cnt);

    // add 2 fields, remove 1st one
    _ssdb->hset(key1, field1, "value1", BinlogCommand::HSET);
    _ssdb->hset(key1, field2, "value2", BinlogCommand::HSET);
    ret = _ssdb->hdel(key1, field1, BinlogCommand::HDEL);
    cnt = _ssdb->hsize(key1);
    ASSERT_EQ(1, cnt);

    _ssdb->hclear(key1);
    cnt = _ssdb->hsize(key1);
    ASSERT_EQ(0, cnt);
}

TEST_F(THashTest, ListKeys) {

    std::string key1 = "key1", field1 = "field1";
    std::string key2 = "key2";
    std::vector<std::string> names;
    _ssdb->hlist(key1, key2, 100, &names);
    ASSERT_EQ(0, names.size());
    
    _ssdb->hset(key1, field1, "value1", BinlogCommand::HSET);
    _ssdb->hset(key2, field1, "value1", BinlogCommand::HSET);
    
    _ssdb->hlist(key1, key2, 100, &names);
    ASSERT_EQ(2, names.size());
    ASSERT_TRUE(names[0] == "key1");
    ASSERT_TRUE(names[1] == "key2");
}

TEST_F(THashTest, Scan) {
    std::string key1 = "key1",
	field1 = "field1", field2 = "field2",
	value1 = "value1", value2 = "value2";
    
    int ret = _ssdb->hset(key1, field1, value1, BinlogCommand::HSET);
    _ssdb->hset(key1, field2, value2, BinlogCommand::HSET);

    HIterator* iter = _ssdb->hscan(key1, "", "", 100);
    std::vector<StrPair> arr;
    while (iter->next()) {
	arr.push_back(std::make_pair(iter->_field, iter->_value));
    }

    ASSERT_EQ(2, arr.size());
    ASSERT_TRUE(arr[0].first == field1);
    ASSERT_TRUE(arr[0].second == value1);
    ASSERT_TRUE(arr[1].first == field2);
    ASSERT_TRUE(arr[1].second == value2);

    // empty
    iter = _ssdb->hscan("not exist", "", "", 100);
    ASSERT_FALSE(iter->next());
}

//TEST(ValueCntTest, BaseTest) {
void foo0() {
    std::string prev_value, key, field, value;
    std::string output, expected;
    {
	// from empty set
	prev_value = key = field = value = output = expected = "";
	int cnt = TEST_get_hash_value_count(key);
	ASSERT_EQ(cnt, 0);
    }
    
    {
	prev_value = field = value = output = expected = "";
	prev_value = ""; field = "a3b4"; value = "112";
	expected.push_back((char)field.length()); expected += field; expected.push_back(':');
	expected.push_back((char)value.length()); expected += value;
	TEST_insert_update_hash_value(prev_value, field, value, &output);
	int cnt = TEST_get_hash_value_count(output);
	ASSERT_EQ(cnt, 1);
	
	prev_value = output; field = "a3b3"; value = "-11159";
	expected.push_back(';');
	expected.push_back((char)field.length()); expected += field; expected.push_back(':');
	expected.push_back((char)value.length()); expected += value;
	TEST_insert_update_hash_value(prev_value, field, value, &output);

	EXPECT_EQ(expected, output);
	cnt = TEST_get_hash_value_count(output);
	ASSERT_EQ(cnt, 2);
    }
}

//TEST(InsertUpdateTest, BaseTest) {
void foo1() {
    std::string prev_value, field, value;
    std::string output, expected;
    {
	// from empty value
	prev_value = field = value = output = expected = "";
	prev_value = ""; field = "a3b4"; value = "112";
	expected.push_back((char)field.length()); expected += field; expected.push_back(':');
	expected.push_back((char)value.length()); expected += value;
	int ret = TEST_insert_update_hash_value(prev_value, field, value, &output);

	EXPECT_NE("a3b4:112", output);
	EXPECT_EQ(expected, output);
    }
    {  
	// insert
	prev_value = field = value = output = expected = "";
	prev_value = ""; field = "a3b4"; value = "112";
	expected.push_back((char)field.length()); expected += field; expected.push_back(':');
	expected.push_back((char)value.length()); expected += value;
	TEST_insert_update_hash_value(prev_value, field, value, &output);

	prev_value = output; field = "a3b3"; value = "-11159";
	expected.push_back(';');
	expected.push_back((char)field.length()); expected += field; expected.push_back(':');
	expected.push_back((char)value.length()); expected += value;
	TEST_insert_update_hash_value(prev_value, field, value, &output);

	EXPECT_EQ(expected, output);
    }
    {
	// update 1
	prev_value = field = value = output = expected = "";
	prev_value = ""; field = "a3b4"; value = "112";
	TEST_insert_update_hash_value(prev_value, field, value, &output);
	
	prev_value = output; field = "a3b4"; value = "-11159";
	expected.push_back((char)field.length()); expected += field; expected.push_back(':');
	expected.push_back((char)value.length()); expected += value;
	TEST_insert_update_hash_value(prev_value, field, value, &output);

	EXPECT_EQ(expected, output);
    }
    {  
	// update 2
	prev_value = field = value = output = expected = "";
	prev_value = ""; field = "a3b4"; value = "112";
	expected.push_back((char)field.length()); expected += field; expected.push_back(':');
	expected.push_back((char)value.length()); expected += value;
	TEST_insert_update_hash_value(prev_value, field, value, &output);

	prev_value = output; field = "a3b3"; value = "-11159";
	TEST_insert_update_hash_value(prev_value, field, value, &output);

	prev_value = output; field = "a3b3"; value = "1724";
	expected.push_back(';');
	expected.push_back((char)field.length()); expected += field; expected.push_back(':');
	expected.push_back((char)value.length()); expected += value;

	int ret = TEST_insert_update_hash_value(prev_value, field, value, &output);
	EXPECT_EQ(expected, output);
    }
}

//TEST(RemoveTest, BaseTest) {
void foo2() {
    std::string prev_value, field, value;
    std::string output, expected;
    {
	// remove none-exist 1
	prev_value = field = value = output = expected = "";
	prev_value = ""; field = "a3b4"; value = "112";
	expected = "";
	int ret = TEST_remove_hash_value(prev_value, field, &output);

	ASSERT_EQ(0, ret);
	EXPECT_EQ(expected, output);
    }
    {
	// remove none-exist 2
	prev_value = field = value = output = expected = "";
	prev_value = ""; field = "a3b4"; value = "112";
	expected.push_back((char)field.length()); expected += field; expected.push_back(':');
	expected.push_back((char)value.length()); expected += value;
	TEST_insert_update_hash_value(prev_value, field, value, &output);

	prev_value = output; field = "sdfsafds";
	int ret = TEST_remove_hash_value(prev_value, field, &output);

	ASSERT_EQ(0, ret);
	EXPECT_EQ(expected, output);
    }
    {
	// remove exist 1
	prev_value = field = value = output = expected = "";
	prev_value = ""; field = "a3b4"; value = "112";
	expected = "";
	TEST_insert_update_hash_value(prev_value, field, value, &output);

	prev_value = output;
	int ret = TEST_remove_hash_value(prev_value, field, &output);

	ASSERT_EQ(1, ret);
	EXPECT_EQ(expected, output);
    }
    {
	// remove exist 2: remove tail
	prev_value = field = value = output = expected = "";
	prev_value = ""; field = "a3b4"; value = "112";
	expected.push_back((char)field.length()); expected += field; expected.push_back(':');
	expected.push_back((char)value.length()); expected += value;
	TEST_insert_update_hash_value(prev_value, field, value, &output);

	prev_value = output; field = "u3y4"; value = "-112";
	TEST_insert_update_hash_value(prev_value, field, value, &output);

	prev_value = output;
	int ret = TEST_remove_hash_value(prev_value, field, &output);

	
	EXPECT_EQ(expected, output);
    }
    {
	// remove exist 3: remove head
	prev_value = field = value = output = expected = "";
	prev_value = ""; field = "a3b4"; value = "112";
	TEST_insert_update_hash_value(prev_value, field, value, &output);

	prev_value = output; field = "u3y4"; value = "-112";
	expected.push_back((char)field.length()); expected += field; expected.push_back(':');
	expected.push_back((char)value.length()); expected += value;
	TEST_insert_update_hash_value(prev_value, field, value, &output);

	prev_value = output; field = "a3b4";
	int ret = TEST_remove_hash_value(prev_value, field, &output);

	ASSERT_EQ(1, ret);
	EXPECT_EQ(expected, output);
    }
}

    
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


