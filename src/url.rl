%%{
    machine url;
  
    # Copied from the URL RFC - https://www.ietf.org/rfc/rfc1738.txt
    # For ragel, we've reversed the order of the url elements

    # ; Miscellaneous definitions
    # 
    # lowalpha       = "a" | "b" | "c" | "d" | "e" | "f" | "g" | "h" |
    #                  "i" | "j" | "k" | "l" | "m" | "n" | "o" | "p" |
    #                  "q" | "r" | "s" | "t" | "u" | "v" | "w" | "x" |
    #                  "y" | "z"
    ##lowalpha = [a-z];
    # hialpha        = "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" |
    #                  "J" | "K" | "L" | "M" | "N" | "O" | "P" | "Q" | "R" |
    #                  "S" | "T" | "U" | "V" | "W" | "X" | "Y" | "Z"
    ##hialpha = [A-Z];

    # alpha          = lowalpha | hialpha
    #alpha          = lowalpha | hialpha; (not needed because it's already part of ragel)
    # digit          = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" |
    #                  "8" | "9"
    #digit = [0-9]; (not needed because it's already part of ragel)
    # safe           = "$" | "-" | "_" | "." | "+"
    safe           = [$\-_.+];
    # extra          = "!" | "*" | "'" | "(" | ")" | ","
    extra          = [!*'(),];
    # national       = "{" | "}" | "|" | "\" | "^" | "~" | "[" | "]" | "`"
    national       = [{}|\^~\[\]`];
    # punctuation    = "<" | ">" | "#" | "%" | <">
    punctuation    = [<>#%"];

    # reserved       = ";" | "/" | "?" | ":" | "@" | "&" | "="
    reserved       = [;/?:@&=];
    # hex            = digit | "A" | "B" | "C" | "D" | "E" | "F" |
    #                  "a" | "b" | "c" | "d" | "e" | "f"
    hex = digit | [A-F] | [a-f];
    # escape         = "%" hex hex
    #escape         = "%" % escape_start hex hex % escape_end;
    escape          = '%';

    # unreserved     = alpha | digit | safe | extra
    unreserved     = alpha | digit | safe | extra;
    # uchar          = unreserved | escape
    uchar          = unreserved | escape;
    # xchar          = unreserved | reserved | escape
    xchar          = unreserved | reserved | escape;
    # digits         = 1*digit
    digits         = digit+;

    # ; URL schemeparts for ip based protocols:
    # 
    # port           = digits
    port           = digits;

    # urlpath        = *xchar    ; depends on protocol see section 3.1
    urlpath        = xchar*; 

    # Because we're not a parser police, and just want to get the info, we're not super strict on the hostname format
    hostname = ( alnum | [0-9] | [\-.])+;
    # toplabel       = alpha | alpha *[ alnum | "-" ] alnum
    #toplabel       = alpha | alpha ( alnum | "-" )* alnum;
    # domainlabel    = alnum | alnum *[ alnum | "-" ] alnum
    #domainlabel    = alnum | alnum ( alnum | "-" )* alnum;
    # hostnumber     = digits "." digits "." digits "." digits
    #hostnumber     = digits "." digits "." digits "." digits;
    # hostname       = *[ domainlabel "." ] toplabel
    #hostname       =  ( domainlabel "." )* toplabel;

    # host           = hostname | hostnumber
    #host           = ( hostname | hostnumber) > host_start % host_end;
    host           = hostname > host_start % host_end;

    # hostport       = host [ ":" port ]
    hostport       = host ( ":" port > port_start % port_end )?;

    # user           = *[ uchar | ";" | "?" | "&" | "=" ]
    user           = ( uchar | [;?&=])+;

    # password       = *[ uchar | ";" | "?" | "&" | "=" ]
    password       = ( uchar | [;?&=])+;

    # login          = [ user [ ":" password ] "@" ] hostport
    login          = ( user > user_start % user_end
                       ( ":" password > password_start % password_end )?
                     "@" );

    # ; The predefined schemes:
    # 


    # ; HTTP
    # 
    # hsegment       = *[ uchar | ";" | ":" | "@" | "&" | "=" ]
    hsegment       = ( uchar | [;:@&=] )*;
    # hpath          = hsegment *[ "/" hsegment ]
    hpath          = hsegment ( "/" hsegment )*;
    # search         = *[ uchar | ";" | ":" | "@" | "&" | "=" ]
    search         = ( uchar | [;:@&=] )* > search_start % search_end;
    scheme         = ("http://" | "https://") % scheme_end;

    # ; Specific predefined schemes are defined here; new schemes
    # ; may be registered with IANA
    # 
    # url            = httpurl | ftpurl | newsurl |
    #                  nntpurl | telneturl | gopherurl |
    #                  waisurl | mailtourl | fileurl |
    #                  prosperourl | otherurl
    # 
   url := scheme login? hostport ( "/" > path_start hpath % path_end ( "?" search )? )?;

}%%
