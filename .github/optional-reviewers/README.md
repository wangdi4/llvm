* Overview

Optional reviewer is a feature similar to customized hearld rules in Phabricator.
It provide a way to map from source files touched in PR to folks who are interested in learning more about them.
Developers can add customized rules to be requested as optional reviewer.
Note that this is different from CODEOWNERS, reviewers is optional and are not authorized to approve PR.

GitHub CI will call get_reviewers.py with the files touched in any new PR ,
get_reviewers.py will then go through all the rules in config.json to determine who should be requested for review.

* Format of config.json

config.json is a json file , the whole file contains an array of rules.
Each rule contains two major keys: "ids" and "match".
"match" is a Python regex expression.
"ids" is an array of id to be invited when the file touched in PR match the Python regex.

* How to use it

Please go through the rules in config.json to see whether there are existing rules matching your expectation,
if so, you can simply add your own id into the "ids" array in those rules.
If not, then you can add a new rules.

Some simple example are givein in config.example.json.

* Test

If you change the config.json, it is recommended to edit testfiles to test the rules.
You can add some files names that should hit your new rules and some that shouldn't.
Then run the python script with -v to make sure your edit is good.

./get_reviewers.py -f testfiles -v




