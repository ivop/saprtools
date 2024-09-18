#! /bin/sh

# Parameters: file_converter file_source file_author_file_title file_name [file_frequency]
create_output() {
  local file_converter=$1
  local file_source=$2
  local file_author=$3
  local file_title=$4
  local file_name=$5.xex
  local file_frequency=$6
  local file_frequency=${file_frequency:-"50"}

  message="Creating title '${file_title}' by '${file_author}' from '${file_source}' as '${file_name}' with player '${player}' at ${file_frequency} Hz."
  echo "$message"
  echo "$message" >>convertall.log
  make "compress-${player}" 2>&1 | tee -a convertall.log
  
  printf 'Source: %-32sTitle : %-32sAuthor: %s' "${file_source}" "${file_title}" "${file_author}" >songname.txt

  make "player${file_frequency}-${player}" >>convertall.log  2>&1
  local file_path="../xex/${file_converter}/${player}"
  mkdir -p "${file_path}"
  mv player.xex "${file_path}/${file_name}" >>convertall.log  2>&1
}

# Evironment: file_source file_author
# Parameters: file_title file_name
create_title() {
  create_output "${file_converter}" "${file_source}" "${file_author}" "$1" "$2" "$3"
}
