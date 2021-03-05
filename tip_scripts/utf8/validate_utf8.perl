#!/usr/bin/perl

if ($#ARGV != 0) {
    print "usage: validate_utf8 <string>\n";
    exit;
}

$test = $ARGV[0];

if($test =~
  /\A(
     [\x00-\x7F]      				      # ASCII
   | [\xC2-\xDF][\x80-\xBF]             # non-overlong 2-byte
   |  \xE0[\xA0-\xBF][\x80-\xBF]        # excluding overlongs
   | [\xE1-\xEC\xEE\xEF][\x80-\xBF]{2}  # straight 3-byte
   |  \xED[\x80-\x9F][\x80-\xBF]        # excluding surrogates
   |  \xF0[\x90-\xBF][\x80-\xBF]{2}     # planes 1-3
   | [\xF1-\xF3][\x80-\xBF]{3}          # planes 4-15
   |  \xF4[\x80-\x8F][\x80-\xBF]{2}     # plane 16
  )*\z/x) {
    print "TRUE\n";
} else {
    print "FALSE\n";
}
