
alias aicp='sh /home/mlamkadm/ai-repos/ai-project-loader.sh'

aicp -d ../cpp-agent-mk1/externals -f tools.md --force
aicp -d ../cpp-agent-mk1/inc -f inc.md --force
aicp -d ../cpp-agent-mk1/src -f src.md --force


AGREGATED_FILES=$(cat tools.md inc.md src.md)

echo "$AGREGATED_FILES" > ../cpp-agent-mk1/context.md
