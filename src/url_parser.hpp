
#line 1 "url_parser.hpp.rl"
#pragma once

#include "logging.hpp"

namespace {

#line 10 "url_parser.hpp"
static const char _url_actions[] = {
	0, 1, 1, 1, 2, 1, 3, 1, 
	4, 1, 6, 1, 7, 1, 8, 1, 
	10, 1, 11, 1, 12, 2, 0, 5, 
	2, 2, 9, 2, 4, 9, 2, 6, 
	2, 2, 7, 3, 2, 11, 12, 3, 
	0, 5, 1
};

static const unsigned char _url_key_offsets[] = {
	0, 0, 1, 2, 3, 4, 6, 7, 
	8, 23, 36, 49, 62, 70, 72, 85, 
	86, 94, 104, 115, 126, 129, 145
};

static const char _url_trans_keys[] = {
	104, 116, 116, 112, 58, 115, 47, 47, 
	33, 59, 61, 63, 95, 36, 44, 45, 
	46, 48, 57, 65, 90, 97, 122, 33, 
	58, 61, 64, 95, 36, 46, 48, 59, 
	63, 90, 97, 122, 33, 59, 61, 63, 
	95, 36, 46, 48, 57, 65, 90, 97, 
	122, 33, 59, 61, 64, 95, 36, 46, 
	48, 57, 63, 90, 97, 122, 45, 46, 
	48, 57, 65, 90, 97, 122, 48, 57, 
	33, 59, 61, 63, 95, 36, 46, 48, 
	57, 65, 90, 97, 122, 58, 47, 58, 
	45, 57, 65, 90, 97, 122, 33, 61, 
	63, 95, 36, 59, 64, 90, 97, 122, 
	33, 61, 95, 36, 46, 48, 59, 64, 
	90, 97, 122, 33, 61, 95, 36, 46, 
	48, 59, 64, 90, 97, 122, 47, 48, 
	57, 33, 47, 58, 59, 61, 63, 64, 
	95, 36, 44, 45, 57, 65, 90, 97, 
	122, 33, 47, 59, 61, 64, 95, 36, 
	46, 48, 57, 63, 90, 97, 122, 0
};

static const char _url_single_lengths[] = {
	0, 1, 1, 1, 1, 2, 1, 1, 
	5, 5, 5, 5, 0, 0, 5, 1, 
	2, 4, 3, 3, 1, 8, 6
};

static const char _url_range_lengths[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	5, 4, 4, 4, 4, 1, 4, 0, 
	3, 3, 4, 4, 1, 4, 4
};

static const unsigned char _url_index_offsets[] = {
	0, 0, 2, 4, 6, 8, 11, 13, 
	15, 26, 36, 46, 56, 61, 63, 73, 
	75, 81, 89, 97, 105, 108, 121
};

static const char _url_indicies[] = {
	0, 1, 2, 1, 3, 1, 4, 1, 
	5, 6, 1, 7, 1, 8, 1, 9, 
	9, 9, 9, 9, 9, 10, 10, 10, 
	10, 1, 11, 12, 11, 13, 11, 11, 
	11, 11, 11, 1, 14, 14, 14, 14, 
	14, 14, 14, 14, 14, 1, 15, 15, 
	15, 16, 15, 15, 15, 15, 15, 1, 
	17, 17, 17, 17, 1, 18, 1, 14, 
	14, 14, 14, 14, 14, 19, 14, 14, 
	1, 5, 1, 21, 22, 20, 20, 20, 
	1, 23, 23, 24, 23, 23, 23, 23, 
	1, 25, 25, 25, 25, 25, 25, 25, 
	1, 26, 26, 26, 26, 26, 26, 26, 
	1, 27, 28, 1, 11, 21, 30, 11, 
	11, 11, 13, 11, 11, 29, 29, 29, 
	1, 15, 27, 15, 15, 16, 15, 15, 
	31, 15, 15, 1, 0
};

static const char _url_trans_targs[] = {
	2, 0, 3, 4, 5, 6, 15, 7, 
	8, 9, 21, 9, 10, 12, 11, 11, 
	12, 16, 20, 22, 16, 17, 13, 17, 
	18, 19, 19, 17, 20, 21, 14, 22
};

static const char _url_trans_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 21, 39, 0, 9, 9, 11, 0, 
	13, 1, 5, 33, 0, 24, 3, 0, 
	15, 17, 0, 27, 0, 0, 30, 0
};

static const char _url_eof_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	3, 15, 36, 19, 7, 3, 7
};

static const int url_start = 1;
static const int url_first_final = 16;
static const int url_error = 0;

static const int url_en_url = 1;


#line 13 "url_parser.hpp.rl"

}

namespace cdnalizerd {

void URL::parse() {
  auto p = std::begin(raw);
  auto pe = std::end(raw);
  auto eof = pe;
  auto start = raw.begin();
  int cs;
  // init
  
#line 128 "url_parser.hpp"
	{
	cs = url_start;
	}

#line 26 "url_parser.hpp.rl"
  // exec
  
#line 136 "url_parser.hpp"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_keys = _url_trans_keys + _url_key_offsets[cs];
	_trans = _url_index_offsets[cs];

	_klen = _url_single_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (unsigned int)(_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _url_range_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += (unsigned int)((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	_trans = _url_indicies[_trans];
	cs = _url_trans_targs[_trans];

	if ( _url_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _url_actions + _url_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 4 "url_actions.rl"
	{
      scheme = std::string(start, p);
    }
	break;
	case 1:
#line 7 "url_actions.rl"
	{
      start = p;
    }
	break;
	case 2:
#line 10 "url_actions.rl"
	{
      host = std::string(start, p);
    }
	break;
	case 3:
#line 13 "url_actions.rl"
	{
        start = p;
    }
	break;
	case 4:
#line 16 "url_actions.rl"
	{
        port = std::string(start, p);
    }
	break;
	case 5:
#line 19 "url_actions.rl"
	{
        start = p;
    }
	break;
	case 6:
#line 22 "url_actions.rl"
	{
        user = std::string(start, p);
    }
	break;
	case 7:
#line 25 "url_actions.rl"
	{
        start = p;
    }
	break;
	case 8:
#line 28 "url_actions.rl"
	{
        password = std::string(start, p);
    }
	break;
	case 9:
#line 31 "url_actions.rl"
	{
        start = p;
    }
	break;
	case 10:
#line 34 "url_actions.rl"
	{
        path = std::string(start, p);
    }
	break;
	case 11:
#line 37 "url_actions.rl"
	{
        start = p;
    }
	break;
#line 282 "url_parser.hpp"
		}
	}

_again:
	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	if ( p == eof )
	{
	const char *__acts = _url_actions + _url_eof_actions[cs];
	unsigned int __nacts = (unsigned int) *__acts++;
	while ( __nacts-- > 0 ) {
		switch ( *__acts++ ) {
	case 2:
#line 10 "url_actions.rl"
	{
      host = std::string(start, p);
    }
	break;
	case 4:
#line 16 "url_actions.rl"
	{
        port = std::string(start, p);
    }
	break;
	case 10:
#line 34 "url_actions.rl"
	{
        path = std::string(start, p);
    }
	break;
	case 11:
#line 37 "url_actions.rl"
	{
        start = p;
    }
	break;
	case 12:
#line 40 "url_actions.rl"
	{
        search = std::string(start, p);
    }
	break;
#line 328 "url_parser.hpp"
		}
	}
	}

	_out: {}
	}

#line 28 "url_parser.hpp.rl"
  // finish
  if (cs < url_first_final) {
    BOOST_THROW_EXCEPTION(
        boost::enable_error_info(std::runtime_error("Unable to parse URL"))
        << err::URL(raw) << err::position(std::distance(std::begin(raw), p)));
  }
  pathAndSearch = path + search;
}
  
} /* cdnalizerd  */ 
