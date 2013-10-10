# copy this file to ~/.config/fish/functions
function ondir_prompt_hook --on-event fish_prompt
    if test ! -e "$OLDONDIRWD"; set -g OLDONDIRWD /; end;
    if [ "$OLDONDIRWD" != "$PWD" ]; eval (ondir $OLDONDIRWD $PWD); end;
    set -g OLDONDIRWD "$PWD";
end
