
alias aicp='sh /home/mlamkadm/ai-repos/ai-project-loader.sh'

rm -f  tools.md inc.md src.md context.md

aicp -d ../agent-lib/externals -f tools.md --force
aicp -d ../agent-lib/inc -f inc.md --force
aicp -d ../agent-lib/src -f src.md --force


AGREGATED_FILES=$(cat tools.md inc.md src.md)

echo "$AGREGATED_FILES" > ../agent-lib/context.md
