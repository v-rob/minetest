// SPDX-License-Identifier: LGPL-2.1-or-later

#include "test.h"
#include "log_internal.h"

using std::ostream;

class TestLogging : public TestBase
{
public:
	TestLogging() { TestManager::registerTestModule(this); }
	const char *getName() { return "TestLogging"; }

	void runTests(IGameDef *gamedef);

	void testNullChecks();
	void testBitCheck();
};

static TestLogging g_test_instance;

void TestLogging::runTests(IGameDef *gamedef)
{
	TEST(testNullChecks);
	TEST(testBitCheck);
}

////////////////////////////////////////////////////////////////////////////////

void TestLogging::testNullChecks()
{
	CaptureLogOutput capture(g_logger);

	infostream << "Test char*: "          << (char*)0          << std::endl;
	infostream << "Test signed char*: "   << (signed char*)0   << std::endl;
	infostream << "Test unsigned char*: " << (unsigned char*)0 << std::endl;

	infostream << "Test const char*: "          << (const char*)0          << std::endl;
	infostream << "Test const signed char*: "   << (const signed char*)0   << std::endl;
	infostream << "Test const unsigned char*: " << (const unsigned char*)0 << std::endl;


	auto logs = capture.take();
	UASSERT_EQ(logs.size(), 6U);
	UASSERT_EQ(logs[0].text, "Test char*: (null)");
	UASSERT_EQ(logs[1].text, "Test signed char*: (null)");
	UASSERT_EQ(logs[2].text, "Test unsigned char*: (null)");
	UASSERT_EQ(logs[3].text, "Test const char*: (null)");
	UASSERT_EQ(logs[4].text, "Test const signed char*: (null)");
	UASSERT_EQ(logs[5].text, "Test const unsigned char*: (null)");
}

namespace {
	class ForceEofBit {};
	class ForceFailBit {};
	class ForceBadBit {};

	ostream& operator<<(ostream& os, ForceEofBit)
	{
		os.setstate(std::ios::eofbit);
		return os;
	}

	ostream& operator<<(ostream& os, ForceFailBit)
	{
		os.setstate(std::ios::failbit);
		return os;
	}

	ostream& operator<<(ostream& os, ForceBadBit)
	{
		os.setstate(std::ios::badbit);
		return os;
	}
}

void TestLogging::testBitCheck()
{
	CaptureLogOutput capture(g_logger);

	infostream << "EOF is " << ForceEofBit{} << std::endl;
	infostream << "Fail is " << ForceFailBit{} << std::endl;
	infostream << "Bad is " << ForceBadBit{} << std::endl;

	auto logs = capture.take();
	UASSERT_EQ(logs.size(), 3U);
	UASSERT_EQ(logs[0].text, "EOF is (ostream:eofbit)");
	UASSERT_EQ(logs[1].text, "Fail is (ostream:failbit)");
	UASSERT_EQ(logs[2].text, "Bad is (ostream:badbit)");
}
