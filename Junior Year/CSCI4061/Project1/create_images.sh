#!/bin/csh
# CSci4061 Spring 2017 Assignment 1 
# Name: Isaac Schwab, Nathan Kaufman
# Student ID: 4841729, 4872400
# CSELabs machine: 4-250-15
# Additional comments 

set noglob
set home = `pwd`
set in_dir = "$home/$1"
set out_dir = "$home/$2"
set thumbs = "$out_dir/thumbs"
set in_pattern = ($*)

# Checks to make sure there are a valid number of arguments
if ("$#" <= 2) then
    echo "Wrong number of arguments"
    echo "Usage: ./create_images.sh <input_directory> <output_directory> <pattern1> <pattern2> ..."
    echo "Example Usage: ./create_images.sh input_dir output_dir *.png *.tiff *.gif"
    echo "Exiting script"
    exit 1
endif
shift in_pattern
shift in_pattern

echo ""
echo "Script starting"
echo "Verifying script arguments..."
# Handles checking the input directory argument
echo "Checking input directory argument: $1"
if ("$1" == "") then
    echo "Error: missing input directory"
    echo "Exiting script"
    exit 1
else if (-d "$in_dir") then
    echo "Directory exists"
else
    echo "Error: $in_dir directory doesn't exist" 
    echo "Exiting script"
    exit 1
endif
echo ""

# Handles checking the output directory argument
echo "Checking output directory argument: $2"
if ("$2" == "") then
    echo "Error: missing output directory"
    echo "Exiting script"
    exit 1
else if (-d "$out_dir") then
    echo "Directory exists"
    echo "Checking for /thumbs directory"
    if (-d "$thumbs") then
        echo "/thumbs exists"
    else
        cd $out_dir
        mkdir thumbs
        cd ..
    endif
else
    echo "Output directory doesn't exist: creating directory $2"
    mkdir $2
    cd $2
    echo "Creating thumbs directory"
    mkdir thumbs
    cd .. 
endif
echo ""


# Get the files in the input directory that meet the pattern arguments
echo "Verifying valid pattern arguments..."
foreach pattern ($in_pattern)
   # Checks the patterns against the valid patterns
    if ("$pattern" =~ "*.png" || "$pattern" =~ "*.tiff" || "$pattern" =~ "*.gif") then
        echo "$pattern is valid"
    else
        echo "Pattern: $pattern is invalid. Exiting script"
        exit 1
    endif 
   # Adds files in the input directory to log file for comparison later
    find "$in_dir" -type f -name $pattern >> in_dir.txt
end


# Get the files currently in the output_dir and add them to a log file
find "$out_dir" -type f -name "*.jpg" > out_dir.txt

echo ""
echo "Processing images..."
# read from out_dir.txt file, then convert to jpg and create thumb version
# if the file already exists in the output directory then skip it
foreach line (`cat in_dir.txt`)  # reads in files from the in_dir.txt file
    set file_name = `basename "$line" | cut -f 1 -d '.'`  # grabs just the filename from the full path
    set upper_name = `echo $file_name | tr "[:lower:]" "[:upper:]"`  # converts the case of the filename

    # Check to make sure file doesn't already exist in output directory
    cd "$out_dir"
    if ( -f "$upper_name.jpg") then
        echo "$upper_name.jpg already exists... Skipping"
    else if (-f "$thumbs/$upper_name""_thumb.jpg") then
        echo "$upper_name""_thumb.jpg already exists... Skipping"
    else
        echo "Converting $line to $upper_name.jpg"
        convert $line "$out_dir/$upper_name.jpg"
        echo "Resizing $line to $upper_name""_thumb.jpg"
        convert $line -resize 200x200 "$thumbs/$upper_name""_thumb.jpg"
    endif
end
echo ""


# Clean up log files
echo "Cleaning up log files..."
cd ..
if ( -f "in_dir.txt") then 
    rm in_dir.txt
endif
if ( -f "out_dir.txt") then 
    rm out_dir.txt
endif
echo ""


echo "Building HTML file..."
##### HTML CODE ######
set html = "$home/pic_name_xx.html"
echo "<html><head><title>Test Images</title></head><body>" > $html
echo "<table border = 2>" >> $html
echo "<tr>" >> $html
#thumbnails + image links
foreach image (`find  $out_dir -maxdepth 1 -type f -name "*.jpg"`)
    echo " <td><a href="\"$image\"">" >> $html
    set thumb_name = `basename "$image" | cut -f 1 -d '.'`
    echo "     <img src ="\"$thumbs/$thumb_name""_thumb.jpg\""/></a>" >> $html
end
echo " </td>" >> $html
echo "</tr>" >> $html
echo "<tr>" >> $html
#resolution
foreach size (`find  $thumbs -maxdepth 1 -type f -name "*.jpg"`)
    set dimensions = `identify -format "%w x %h" $size`
    echo " <td align="\"center\""> $dimensions </td>" >> $html
end
echo "</tr>" >> $html
echo "<tr>" >> $html
#date modified
foreach date (`find  $out_dir -maxdepth 1 -type f -name "*.jpg"`)
    set moddate = `stat -c %y $date | cut -f 1 -d ' '` 
    echo " <td align ="\"center\""> $moddate </td>" >> $html
end
echo "</tr>" >> $html
echo "</table>" >> $html
#User Theme/message
echo "Enter your user theme message:"
set theme = $<
echo "<p> Theme: < $theme > </p>" >> $html
#Current date
set curdate = `date | cut -f 1-4,7 -d ' '`
echo "<p> Date & Day: < $curdate > </p>" >> $html
echo "</body></html>" >> $html

echo "Script complete!"




