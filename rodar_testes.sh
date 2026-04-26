#!/bin/bash

# ==============================================================================
# SCRIPT DE COMPILAÇÃO E EXECUÇÃO AUTOMATIZADA - MPI
# ==============================================================================

N=500000000
ARQUIVO_SAIDA="resultados_finais.txt"

# Limpa o arquivo de saída se ele já existir e cria um cabeçalho
echo "============================================================" > $ARQUIVO_SAIDA
echo "       RESULTADOS DAS EXECUÇÕES - NÚMEROS PRIMOS (N=$N)     " >> $ARQUIVO_SAIDA
echo "============================================================" >> $ARQUIVO_SAIDA
echo "" >> $ARQUIVO_SAIDA

# Lista das pastas a serem varridas
PASTAS=("naive" "bag")

for pasta in "${PASTAS[@]}"; do
    if [ -d "$pasta" ]; then
        echo "[+] Processando a pasta: ./$pasta/"
        
        # Procura todos os arquivos .c dentro da pasta
        for arquivo_c in $pasta/*.c; do
            
            # Verifica se encontrou algum arquivo para evitar erros caso a pasta esteja vazia
            if [ -f "$arquivo_c" ]; then
                
                # Extrai apenas o nome do arquivo sem a extensão .c
                nome_base=$(basename "$arquivo_c" .c)
                executavel="$pasta/$nome_base.out"
                
                echo "    -> Compilando: $arquivo_c"
                mpicc "$arquivo_c" -o "$executavel" -lm
                
                if [ $? -eq 0 ]; then
                    echo "------------------------------------------------------------" >> $ARQUIVO_SAIDA
                    echo "MÉTODO: $pasta / $nome_base" >> $ARQUIVO_SAIDA
                    echo "------------------------------------------------------------" >> $ARQUIVO_SAIDA
                    
                    # Definição dos processos base dependendo da abordagem
                    if [ "$pasta" == "naive" ]; then
                        proc_base=1
                    else
                        proc_base=2 # Bag of tasks exige no mínimo 2 (1 Mestre + 1 Trabalhador)
                    fi

                    # Execução com o número base de processos (1 ou 2)
                    echo "       Rodando com $proc_base processo(s)..."
                    echo ">>> TESTE: $proc_base PROCESSO(S)" >> $ARQUIVO_SAIDA
                    mpiexec -n $proc_base "$executavel" $N >> $ARQUIVO_SAIDA 2>&1
                    
                    # Execução com 4 processos
                    echo "       Rodando com 4 processos..."
                    echo ">>> TESTE: 4 PROCESSOS" >> $ARQUIVO_SAIDA
                    mpiexec -n 4 "$executavel" $N >> $ARQUIVO_SAIDA 2>&1
                    echo "" >> $ARQUIVO_SAIDA

                    # (Opcional) Remove o executável após o uso para deixar a pasta limpa
                    rm "$executavel"
                else
                    echo "[!] ERRO na compilação de $arquivo_c" | tee -a $ARQUIVO_SAIDA
                fi
            fi
        done
    else
        echo "[!] A pasta ./$pasta/ não foi encontrada. Pulando..."
    fi
done

echo ""
echo "[+] Bateria de testes finalizada! Verifique o arquivo '$ARQUIVO_SAIDA'."