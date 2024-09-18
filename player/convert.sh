#! /bin/sh

# Parameters: file_source file_author_file_title file_name [file_frequency]
create_output() {
  file_source=$1
  file_author=$2
  file_title=$3
  file_name=$4.xex
  file_frequency=$5
  file_frequency=${file_frequency:-"50"}

  message="Creating title '${file_title}' by '${file_author}' from '${file_source}' as '${file_name}' with player '${player}' at ${file_frequency} Hz."
  echo "$message"
  echo "$message" >>convertall.log
  make "compress${player}" 2>&1 | tee -a convertall.log
  
  printf 'Source: %-32sTitle : %-32sAuthor: %s' "${file_source}" "${file_title}" "${file_author}" >songname.txt

  make "player${file_frequency}${player}" >>convertall.log  2>&1
  mv player.xex "xex${player}/${file_name}" >>convertall.log  2>&1
}

# Evironment: file_source file_author
# Parameters: file_title file_name
create_title() {
  create_output "${file_source}" "${file_author}" "$1" "$2" "$3"
}
