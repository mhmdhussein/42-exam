FILE='print_bits.c'
ASSIGN='print_bits'

bash .system/auto_correc_main.sh $FILE $ASSIGN 2
if [ -e .system/grading/traceback ];then
    mv .system/grading/traceback .
	exit 1
fi

bash .system/auto_correc_main.sh $FILE $ASSIGN 432
if [ -e .system/grading/traceback ];then
    mv .system/grading/traceback .
	exit 1
fi

bash .system/auto_correc_main.sh $FILE $ASSIGN 12
if [ -e .system/grading/traceback ];then
    mv .system/grading/traceback .
	exit 1
fi

bash .system/auto_correc_main.sh $FILE $ASSIGN -2
if [ -e .system/grading/traceback ];then
    mv .system/grading/traceback .
	exit 1
fi

bash .system/auto_correc_main.sh $FILE $ASSIGN 42
if [ -e .system/grading/traceback ];then
    mv .system/grading/traceback .
	exit 1
fi

touch .system/grading/passed;
