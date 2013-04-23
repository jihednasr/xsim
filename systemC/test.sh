#!/bin/bash
TEST=0

function changement_valeur
{
    echo "test avant changement: ${TEST}"
    TEST=$1
    echo "test après changement: ${TEST}"
}


echo "TEST: ${TEST}"
echo "appel à la function"
changement_valeur 2

echo "nouvelle valeur de TEST: ${TEST}"
