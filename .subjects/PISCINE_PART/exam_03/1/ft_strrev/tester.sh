FILE='ft_strrev.c'
ASSIGN='ft_strrev'

bash .system/auto_correc_main.sh $FILE $ASSIGN "string"
if [ -e .system/grading/traceback ];then
    mv .system/grading/traceback .
	exit 1
fi

bash .system/auto_correc_main.sh $FILE $ASSIGN "error"
if [ -e .system/grading/traceback ];then
    mv .system/grading/traceback .
	exit 1
fi

bash .system/auto_correc_main.sh $FILE $ASSIGN "quarante deux"
if [ -e .system/grading/traceback ];then
    mv .system/grading/traceback .
	exit 1
fi

bash .system/auto_correc_main.sh $FILE $ASSIGN "lol"
if [ -e .system/grading/traceback ];then
    mv .system/grading/traceback .
	exit 1
fi

touch .system/grading/passed;
