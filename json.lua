package.path = "./print_table.lua";

require "print_table.lua";

print_table(cmath);
print_table(cjson);

print(cmath.add(1, 2))
print(cmath.sub(1, 2))

jsonstr = [[{
    "quiz": {
        "sport": {
            "q1": {
                "question": "Which one is correct team name in NBA?",
                "options": [
                    "New York Bulls",
                    "Los Angeles Kings",
                    "Golden State Warriros",
                    "Huston Rocket"
                ],
                "answer": "Huston Rocket"
            }
        },
        "maths": {
            "q1": {
                "question": "5 + 7 = ?",
                "options": [
                    "10",
                    "11",
                    "12",
                    "13"
                ],
                "answer": "12"
            },
            "q2": {
                "question": "12 - 8 = ?",
                "options": [
                    "1",
                    "2",
                    "3",
                    "4"
                ],
                "answer": "4"
            }
        }
    }
}]];

ret,jsontab = cjson.parse(jsonstr);
print("ret="..ret);
print_table(jsontab);
