#include "gtest/gtest.h"

#include "bout.hxx"
#include "boutexception.hxx"
#include "test_extras.hxx"
#include "utils.hxx"

#include <algorithm>
#include <csignal>
#include <iostream>
#include <string>
#include <vector>

std::vector<char*> get_c_string_vector(std::vector<std::string>& vec_args) {
  std::vector<char*> c_args{};
  std::transform(begin(vec_args), end(vec_args), std::back_inserter(c_args),
                 [](std::string& arg) { return &arg.front(); });
  return c_args;
}

TEST(ParseCommandLineArgs, HelpShortOption) {
  std::vector<std::string> v_args{"test", "-h"};
  auto c_args = get_c_string_vector(v_args);
  char** argv = c_args.data();

  auto cout_buf = std::cout.rdbuf();
  std::cout.rdbuf(std::cerr.rdbuf());

  EXPECT_EXIT(bout::experimental::parseCommandLineArgs(c_args.size(), argv),
              ::testing::ExitedWithCode(0), _("Usage:"));

  std::cout.rdbuf(cout_buf);
}

TEST(ParseCommandLineArgs, HelpLongOption) {
  std::vector<std::string> v_args{"test", "--help"};
  auto c_args = get_c_string_vector(v_args);
  char** argv = c_args.data();

  auto cout_buf = std::cout.rdbuf();
  std::cout.rdbuf(std::cerr.rdbuf());

  EXPECT_EXIT(bout::experimental::parseCommandLineArgs(c_args.size(), argv),
              ::testing::ExitedWithCode(0), _("Usage:"));

  std::cout.rdbuf(cout_buf);
}

TEST(ParseCommandLineArgs, DataDir) {
  std::vector<std::string> v_args{"test", "-d", "test_data_directory"};
  auto v_args_copy = v_args;
  auto c_args = get_c_string_vector(v_args_copy);
  char** argv = c_args.data();

  auto args = bout::experimental::parseCommandLineArgs(c_args.size(), argv);

  EXPECT_EQ(args.data_dir, "test_data_directory");
  EXPECT_EQ(args.original_argv, v_args);
}

TEST(ParseCommandLineArgs, DataDirBad) {
  std::vector<std::string> v_args{"test", "-d"};
  auto c_args = get_c_string_vector(v_args);
  char** argv = c_args.data();

  EXPECT_THROW(bout::experimental::parseCommandLineArgs(c_args.size(), argv),
               BoutException);
}

TEST(ParseCommandLineArgs, OptionsFile) {
  std::vector<std::string> v_args{"test", "-f", "test_options_file"};
  auto v_args_copy = v_args;
  auto c_args = get_c_string_vector(v_args_copy);
  char** argv = c_args.data();

  auto args = bout::experimental::parseCommandLineArgs(c_args.size(), argv);

  EXPECT_EQ(args.opt_file, "test_options_file");
  EXPECT_EQ(args.original_argv, v_args);
}

TEST(ParseCommandLineArgs, OptionsFileBad) {
  std::vector<std::string> v_args{"test", "-f"};
  auto c_args = get_c_string_vector(v_args);
  char** argv = c_args.data();

  EXPECT_THROW(bout::experimental::parseCommandLineArgs(c_args.size(), argv),
               BoutException);
}

TEST(ParseCommandLineArgs, SettingsFile) {
  std::vector<std::string> v_args{"test", "-o", "test_settings_file"};
  auto v_args_copy = v_args;
  auto c_args = get_c_string_vector(v_args_copy);
  char** argv = c_args.data();

  auto args = bout::experimental::parseCommandLineArgs(c_args.size(), argv);

  EXPECT_EQ(args.set_file, "test_settings_file");
  EXPECT_EQ(args.original_argv, v_args);
}

TEST(ParseCommandLineArgs, SettingsFileBad) {
  std::vector<std::string> v_args{"test", "-o"};
  auto c_args = get_c_string_vector(v_args);
  char** argv = c_args.data();

  EXPECT_THROW(bout::experimental::parseCommandLineArgs(c_args.size(), argv),
               BoutException);
}

TEST(ParseCommandLineArgs, LogFile) {
  std::vector<std::string> v_args{"test", "-l", "test_log_file"};
  auto v_args_copy = v_args;
  auto c_args = get_c_string_vector(v_args_copy);
  char** argv = c_args.data();

  auto args = bout::experimental::parseCommandLineArgs(c_args.size(), argv);

  EXPECT_EQ(args.log_file, "test_log_file");
  EXPECT_EQ(args.original_argv, v_args);
}

TEST(ParseCommandLineArgs, LogFileBad) {
  std::vector<std::string> v_args{"test", "-l"};
  auto c_args = get_c_string_vector(v_args);
  char** argv = c_args.data();

  EXPECT_THROW(bout::experimental::parseCommandLineArgs(c_args.size(), argv),
               BoutException);
}

class PrintStartupTest : public ::testing::Test {
public:
  PrintStartupTest() : sbuf(std::cout.rdbuf()) {
    // Redirect cout to our stringstream buffer or any other ostream
    std::cout.rdbuf(buffer.rdbuf());
  }

  virtual ~PrintStartupTest() {
    // Clear buffer
    buffer.str("");
    // When done redirect cout to its old self
    std::cout.rdbuf(sbuf);
  }

  // Write cout to buffer instead of stdout
  std::stringstream buffer;
  // Save cout's buffer here
  std::streambuf* sbuf;
};

TEST_F(PrintStartupTest, Header) {
  bout::experimental::printStartupHeader(4, 8);

  EXPECT_TRUE(IsSubString(buffer.str(), BOUT_VERSION_STRING));
  EXPECT_TRUE(IsSubString(buffer.str(), _("4 of 8")));
}

TEST_F(PrintStartupTest, CompileTimeOptions) {
  bout::experimental::printCompileTimeOptions();

  EXPECT_TRUE(IsSubString(buffer.str(), _("Compile-time options:\n")));
  EXPECT_TRUE(IsSubString(buffer.str(), _("Signal")));
  EXPECT_TRUE(IsSubString(buffer.str(), "netCDF"));
  EXPECT_TRUE(IsSubString(buffer.str(), "OpenMP"));
  EXPECT_TRUE(IsSubString(buffer.str(), _("Compiled with flags")));
}

TEST_F(PrintStartupTest, CommandLineArguments) {
  std::vector<std::string> args{"-d", "test1", "test2", "test3"};
  bout::experimental::printCommandLineArguments(args);

  for (auto& arg : args) {
    EXPECT_TRUE(IsSubString(buffer.str(), arg));
  }
}

#ifdef SIGHANDLE
class SignalHandlerTest : public ::testing::Test {
public:
  SignalHandlerTest() = default;
  virtual ~SignalHandlerTest() {
    std::signal(SIGUSR1, SIG_DFL);
    std::signal(SIGFPE, SIG_DFL);
    std::signal(SIGSEGV, SIG_DFL);
  }
};

TEST_F(SignalHandlerTest, SegFault) {
  bout::experimental::setupSignalHandler(bout::experimental::defaultSignalHandler);
  // This test is *incredibly* expensive, maybe as much as 1s, so only test the one signal
  EXPECT_DEATH(std::raise(SIGSEGV), "SEGMENTATION FAULT");
}
#endif
