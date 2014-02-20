eval_ondir() {
  eval "`ondir \"$OLDPWD\" \"$PWD\"`"
}
chpwd_functions=( eval_ondir $chpwd_functions )
