#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <numeric>

using namespace std;

namespace data
{
    map<char,int>   var;    //Здесь хранятся переменные
}

void deleteBeginningSpace(string* codeLine)
{

    size_t posBegin = 0;
    while(posBegin < codeLine->length() && isspace(codeLine->at(posBegin)) )
        posBegin++;
    codeLine->erase(0, posBegin);
}

string nextWord(string* codeLine)
{
    deleteBeginningSpace(codeLine);
    size_t posEnd = codeLine->find(' ');
    string theNextWord;
    if(posEnd != string::npos)
        theNextWord = codeLine->substr(0, posEnd);
    else
        theNextWord = *codeLine;
    codeLine->erase(0, theNextWord.length());
    deleteBeginningSpace(codeLine);
    return theNextWord;
}

/* Вычисляет значение выражения */
double calc(const string& arg)
{
    /* Идея
    Делим входные данные на два массива, в первом -- операнды,
    во втором -- операторы. Далее группируем операнды, которые
    соответствуют операции умножения либо сложения, парами и
    вычисляем значение операции. Результат возвращаем на место
    пары n*m в массиве операндов. Делаем это до тех пор, пока
    не закочатся операции умножения и деления. Сложив весь
    преобразованный массив оперндов, получаем результат выражения
    arg */
    /* Вычисляем выражение */
    vector<double>      num;
    string              op;
    string line =       arg;
    for(size_t i = 0; i < line.length(); ++i)
    {
        /* Находим операнды и пушим их в стек чисел */
        try
        { /* Исключение бросает stod в случае, если  первый аргумент не число */
            size_t endPos = 0; // Конец числа
            num.push_back(stod(line.substr(i), &endPos));
            i = i + endPos - 1;

            if(i != line.length() - 1)
                op += '+';
        }
        catch(const invalid_argument&e)
        {
            /* Находим операторы и пушим их в стэк операторов */
            if(line[i] == '*' || line[i] == '/')
            {
                op[op.size() - 1] = line[i];
            }
        }
    }
    /* Калькулируем */
    while(true)
    {
        size_t pos = 0;

        pos = op.find('*');
        if(pos == string::npos)
        {
            pos = op.find('/');
            if(pos == string::npos)
                break;
            else
                num[pos+1] = num[pos]/num[pos+1];
        }
        else
            num[pos+1] = num[pos]*num[pos+1];

        op.erase(pos, 1);
        num.erase(num.begin() + pos);
    }
    double result = 0;
    result = accumulate(num.begin(), num.end(), result);

    return result;
}

/* Вычисляет значение выражения с переменными */
double calc_equation(string input)
{
    /* Замена переменных */
    for(const auto& e: data::var)
    {
        while(true)
        { /* Просматриваем строку столько раз, сколько необходимо для замены всех переменных с именем e.first */
            size_t posVar = input.find(e.first);
            if(posVar != string::npos)
            {
                /* Переменная нашлась. Заменяем её символ на значение */
                string val = to_string(e.second);
                input.replace(posVar, 1, val);
            }
            else break;
        }
    }
    /* Если переменная была со знаком минус, то из 5-X может получиться запись вида 5--X */
    /* Меняем двойной минус на + */
    while(true)
    {
        size_t posOfDMinus = input.find("--");
        if(posOfDMinus != string::npos)
            input.replace(posOfDMinus, 2, "+");
        else
            break;
    }
    return calc(input);
}

/* Проверяет истинность выражения expr */
bool condition_test(string expr)
{
    const static string op[] = {"==", "!=", "<", ">"};

    size_t middle = 0;
    for(const auto& p: op)
    {
        middle = expr.find(p);
        if(middle != string::npos)
        {
            double lval = calc_equation( string{expr, 0, middle} );
            double rval = calc_equation( string{expr, middle+p.length()} );
            if(p == op[0])
            {
                return lval == rval;
            }
            else if(p == op[1])
            {
                return lval != rval;
            }
            else if(p == op[2])
            {
                return lval < rval;
            }
            else if(p == op[3])
            {
                return lval > rval;
            }
        }
    }
    /* Оператор сравнения не найден */
    return false;
}

void cmdLet(string codeLine)
{
    char symbol_var = codeLine[0];
    char symbol_equal = codeLine[1];
    if(symbol_equal == '=')
    {
        string equation = codeLine.substr(2, codeLine.length() - 2);
        data::var[symbol_var] = calc_equation(equation);
        //cout << symbol_var << "=" << data::var[symbol_var] << endl;
    }
}

int main()
{
    cout << "Enter file name: ";
    string fileName;
    cin >> fileName;
    fileName += ".txt";
    ifstream file(fileName);

    vector<string> codeSegment;
    size_t limitLine = 0;
    if(file.is_open())
    {
        char line[256];
        while(file.getline(line, 255, '\n'))
        {
            codeSegment.push_back(string(line));
        }
    }
    else
    {
        cerr << "\nFile " << fileName << " isn't open!" << endl;
        return -1;
    }

    /* Вспомогательные флаги и переменные */
    /* Используются для обработки инструкций в блоках then and else */
    bool fSubCodeLine = false;
    string subCodeLine;

    limitLine = codeSegment.size();
    for(size_t i = 0; i < limitLine; ++i)
    {
        string codeLine = (fSubCodeLine) ? subCodeLine : codeSegment[i];
        fSubCodeLine = false;
        codeLine.erase(0, codeLine.find(' ')); //Удаляем номер строки

        string cmd = nextWord(&codeLine); //В cmd находится первое слово инструкции
        /* Введение новой переменной */
        if(cmd == "let")
        { /* let <var>=<expr> - присваивание */
            //cout << codeLine;
            cmdLet(codeLine);
        }
        /* Присваивание значения существующей переменной */
        else if(cmd.find('=') != string::npos)
        { /* X=5*/
            //cout << "!" << cmd << "!";
            try
            {
                string equation = cmd.substr(cmd.find('='));
                data::var.at(cmd[0]) = calc_equation(equation);
            }
            catch(const out_of_range& e)
            {
                cerr << "Error: " << "the variable " << cmd[0] << " isn't exist" << endl;
                return -1;
            }
        }
        /* Ветвление */
        else if (cmd == "if")
        {
            string condition = nextWord(&codeLine);
            bool bfCondition = condition_test(condition);

            size_t posThen = codeLine.find("then");
            size_t posElse = codeLine.find("else");
            bool bfThenExist = posThen != string::npos;
            bool bfElseExist = posElse != string::npos;

            subCodeLine     = to_string(i) + " "; /* Строка для кода из than или else ветви, объявлена вне цикла */

            if(bfCondition && bfThenExist)
            {
                subCodeLine     = codeLine.substr(posThen + string("then").length(), posElse);
                fSubCodeLine    = true;
            }
            else if (!bfCondition && bfElseExist)
            {
                subCodeLine     = codeLine.substr(posElse + string("else").length());
                fSubCodeLine    = true;
            }

            continue;
        }
        /* Вывод на экран */
        else if(cmd == "print")
        {
            string args = codeLine;

            bool bEndOfPrint = false;
            while(!bEndOfPrint)
            {
                size_t divPos = args.find(','); // Находим позицию запятой-разделителя аргументов
                if(divPos == string::npos)
                {
                    divPos = args.length();
                    bEndOfPrint = true;
                }

                size_t beginPos = 0; // Пропускаем пробелы для выделения начала аргумента функции print
                while( isspace(args[beginPos]) )
                    beginPos++;

                if(args[beginPos] == '\"') // Имеем дело со строкой
                {
                    string msg = args.substr(beginPos+1, divPos - beginPos - 2); // Выделяем строку без ковычек
                    while(true) // Меняем два последовательных символа \n на символ конца строки
                    {
                        size_t nPos = msg.find("\\n");
                        if(nPos == string::npos)
                            break;
                        else
                            msg.replace(nPos, 2, "\n");
                    }
                    cout << msg;
                }
                else // Имеем дело с выражением
                    cout << calc_equation(args.substr(beginPos, divPos - beginPos));

                if(!bEndOfPrint)
                    args = args.substr(divPos+1);
            }

            //cout << expr;
            //system("pause");
        }
        else if(cmd == "goto")
        {
            int arg = stoi(nextWord(&codeLine)); //Получаем номер строки для перехода
            if( arg < 0 || arg >= static_cast<int>(limitLine) )
            {
                cerr << "Error: " << "in goto invalid line number" << endl;
                return -1;
            }
            i = arg - 1;
            //cout << "jmp ";
            //system("pause");
            continue;
        }
        else if(cmd == "exit")
        {
            break;
        }
    }
    return 0;
}
