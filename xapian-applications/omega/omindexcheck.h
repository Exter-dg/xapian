#include <xapian.h>
#include <vector>
#include <string>
#include <unordered_map>

typedef std::vector<std::string> testcase;

class omindexcheck {

  public:
    static std::unordered_map<std::string, testcase> tests;

    // To register pre-defined terms in unordered_map tests
    static void register_tests();
    // To compare terms stored in tests with terms indexed from files
    static bool compare_test(testcase& test,
                             const Xapian::Document& doc,
                             const std::string& file);
    // To make sure specific terms are not present in files
    // No terms with prefix "Z" in blank files
    // No terms with author/title in files without meta data
    static bool check_terms(const Xapian::Document& doc,
                     const std::string& file);
};
