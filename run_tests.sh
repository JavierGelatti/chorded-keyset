#!/bin/bash
#set -euo pipefail

(bundle exec arduino_ci.rb --skip-examples-compilation \
    && printf "\n🟢 \033[92mPASS\033[0m\n") || printf "\n🔴 \033[91mFAIL\033[0m\n"
