<?php


$buildversion = $argv[1];
echo "Build version is $buildversion\n";
$versionflag = "VERSION_GOES_HERE";

if(empty($buildversion)) {
    echo "Could not find build version!";
    exit(-1);
}

$cppfile = "src/globals.h";

echo "Replacing $cppfile script with $buildversion\n";
$str=file_get_contents($cppfile);

$buildversioncommas = str_replace(".", "," , $buildversion).",0";

$str=replace_between($str, "//BEGIN SW VERSION", "//END SW VERSION", "\n#define SW_VERSION \"$buildversion\"\n");
$str=replace_between($str, "//BEGIN FILE VERSION", "//END FILE VERSION", "\n#define VER_FILEVERSION $buildversioncommas\n");

file_put_contents($cppfile, $str);



function replace_between($str, $needle_start, $needle_end, $replacement) {
    $pos = strpos($str, $needle_start);
    $start = $pos === false ? 0 : $pos + strlen($needle_start);

    $pos = strpos($str, $needle_end, $start);
    $end = $pos === false ? strlen($str) : $pos;

    return substr_replace($str, $replacement, $start, $end - $start);
}
