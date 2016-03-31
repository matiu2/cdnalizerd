/** Shows the difference between the user locale and the standard locale  */

#include <iostream>
#include <locale>
#include <iterator>

using namespace std;

int main(int, char **) {
  locale global;
  locale user("");

  // Print locale names
  cout << "Default output Locale: " << cout.getloc().name() << endl
       << "Default input Locale: " << cin.getloc().name() << endl
       << "Global Locale: " << global.name() << endl
       << "User Locale: " << user.name() << endl;

  // Read money
  cout << "Enter the price: ";
  long double m;
  {
    istream::sentry guard(cin);

    auto &money = use_facet<money_get<char>>(user);
    std::ios::iostate state;
    istreambuf_iterator<char> end;
    money.get(cin, end, true, cin, state, m);
  }
  ++m;

  cout << endl << "plus one: ";

  {
    ostream::sentry guard(cout);

    auto &money = use_facet<money_put<char>>(user);
    money.put(cout, true, cout, cout.fill(), m);
  }

  cout << endl;

  return 0;
}

