<?php
$buildversion = trim(file_get_contents("buildversion.txt"));
echo "Replacing version with $buildversion\n";

$cppfile = "src/globals.h";

echo "Replacing $cppfile script with $buildversion\n";
$str=file_get_contents($cppfile);

$str=replace_between($str, "//BEGIN SW VERSION", "//END SW VERSION", "\n#define SW_VERSION \"$buildversion\"\n");

file_put_contents($cppfile, $str);



function replace_between($str, $needle_start, $needle_end, $replacement) {
    $pos = strpos($str, $needle_start);
    $start = $pos === false ? 0 : $pos + strlen($needle_start);

    $pos = strpos($str, $needle_end, $start);
    $end = $pos === false ? strlen($str) : $pos;

    return substr_replace($str, $replacement, $start, $end - $start);
}