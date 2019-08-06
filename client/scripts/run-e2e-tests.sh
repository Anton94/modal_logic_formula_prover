# Start selenium server and trash the verbose error messages from webdriver
webdriver-manager start 2>/dev/null &
# Wait 3 seconds for port 4444 to be listening connections
sleep 5
#  run protractor
protractor ./e2e/conf.js
