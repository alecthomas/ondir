
# Thanks go to Matthew Russell who provided this script. His usage 
# comments follow:
# 
# This could be inserted into either ~/.tcshrc or
# ~/.login. I don't think the "cwdcmd" alias works
# on csh, the forerunner to tcsh, though.
#

# Run ondir on change of directory
alias cwdcmd eval \`ondir -t \$owd\`

# Run ondir on login
eval ondir -t /
