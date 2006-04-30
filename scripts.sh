#
# These functions override builtin BASH commands that change directories.
#
# This script should be added to either the system wide shell initialisation
# file (/etc/profile) or a user specific initialisation file (~/.bash_profile 
# or ~/.profile). In addition, if you are using X, terminals you start up
# should be login terminals (typically -ls, --ls or something to that effect).
#

cd()
{
	builtin cd "$@" && eval "`ondir \"$OLDPWD\" \"$PWD\"`"
}

pushd()
{
	builtin pushd "$@" && eval "`ondir \"$OLDPWD\" \"$PWD\"`"
}

popd()
{
	builtin popd "$@" && eval "`ondir \"$OLDPWD\" \"$PWD\"`"
}              

# Run ondir on login
eval "`ondir /`"
