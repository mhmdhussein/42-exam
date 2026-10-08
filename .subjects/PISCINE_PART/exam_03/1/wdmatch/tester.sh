FILE='wdmatch.c'
ASSIGN='wdmatch'

bash .system/auto_correc_main.sh $FILE $ASSIGN
if [ -e .system/grading/traceback ];then
    mv .system/grading/traceback .
	exit 1
fi

bash .system/auto_correc_main.sh $FILE $ASSIGN "error" rrerrrfiiljdfxjyuifrrvcoojh
if [ -e .system/grading/traceback ];then
    mv .system/grading/traceback .
	exit 1
fi

bash .system/auto_correc_main.sh $FILE $ASSIGN "quarante deux" "qfqfsudf arzgsayns tsregfdgs sjytdekuoixq "
if [ -e .system/grading/traceback ];then
    mv .system/grading/traceback .
	exit 1
fi

bash .system/auto_correc_main.sh $FILE $ASSIGN "faya" "fgvvfdxcacpolhyghbred"
if [ -e .system/grading/traceback ];then
    mv .system/grading/traceback .
	exit 1
fi

bash .system/auto_correc_main.sh $FILE $ASSIGN "faya" "fgvvfdxcacpolhyghbreda"
if [ -e .system/grading/traceback ];then
    mv .system/grading/traceback .
	exit 1
fi

touch .system/grading/passed;
