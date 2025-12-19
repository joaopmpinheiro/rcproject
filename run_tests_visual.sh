#!/bin/bash
# ==============================================================================
# SISTEMA DE TESTES - ES SERVER (via Tejo)
# Projeto Redes de Computadores 2024/2025
# ==============================================================================

# Cores
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
GRAY='\033[0;90m'
NC='\033[0m' # No Color

# Variaveis globais
SERVER_IP=""
SERVER_PORT=0
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0
EXECUTED_TESTS=()

# ==============================================================================
# VALIDACAO DO PROJETO
# ==============================================================================

check_project_folder() {
    echo -e "${CYAN}Verificando ficheiros do projeto...${NC}"
    
    local script_count=0
    local missing_scripts=()
    
    # Verificar alguns scripts essenciais
    for i in 1 8 12 21 25; do
        local script_file=$(printf "script_%02d.txt" $i)
        if [ -f "$script_file" ]; then
            script_count=$((script_count + 1))
        else
            missing_scripts+=("$script_file")
        fi
    done
    
    if [ $script_count -lt 3 ]; then
        echo ""
        echo -e "${RED}  ================================================================${NC}"
        echo -e "${RED}  ERRO - FICHEIROS DE TESTE NAO ENCONTRADOS${NC}"
        echo -e "${RED}  ================================================================${NC}"
        echo ""
        echo -e "${YELLOW}  Este script deve ser executado na pasta do projeto que contem${NC}"
        echo -e "${YELLOW}  os ficheiros de teste: script_01.txt, script_02.txt, etc.${NC}"
        echo ""
        echo -e "${GRAY}  Ficheiros em falta:${NC}"
        for script in "${missing_scripts[@]}"; do
            echo -e "${GRAY}    - $script${NC}"
        done
        echo ""
        echo -e "${RED}  Por favor, execute este script na pasta correta do projeto.${NC}"
        echo ""
        exit 1
    fi
    
    echo -e "${GREEN}OK - Ficheiros de teste encontrados!${NC}"
    echo ""
    sleep 1
}

# ==============================================================================
# FUNCOES DE INTERFACE
# ==============================================================================

show_welcome_menu() {
    clear
    echo ""
    echo -e "${CYAN}  ================================================================${NC}"
    echo -e "${CYAN}  ||                                                            ||${NC}"
    echo -e "${CYAN}  ||   ES TESTS - EVENT RESERVATION SERVER TEST SUITE          ||${NC}"
    echo -e "${CYAN}  ||              (via tejo.tecnico.ulisboa.pt)                 ||${NC}"
    echo -e "${CYAN}  ||                                                            ||${NC}"
    echo -e "${CYAN}  ================================================================${NC}"
    echo ""
    
    echo -e "${YELLOW}  ------------------------------------------------------------------${NC}"
    echo -e "${YELLOW}  CONFIGURACAO DO SERVIDOR ES (onde corre o vosso servidor)${NC}"
    echo -e "${YELLOW}  ------------------------------------------------------------------${NC}"
    echo ""
    
    echo -e "  Escolha o servidor onde corre o ES:"
    echo ""
    echo -e "${GRAY}     [1] 193.136.128.103 (sigma00)${NC}"
    echo -e "${GRAY}     [2] 193.136.128.104 (sigma01)${NC}"
    echo -e "${GRAY}     [3] 193.136.128.105 (sigma02)${NC}"
    echo -e "${GRAY}     [4] 193.136.128.106 (sigma03)${NC}"
    echo -e "${GRAY}     [5] 193.136.128.107 (sigma04)${NC}"
    echo -e "${GRAY}     [6] 193.136.128.108 (sigma05)${NC}"
    echo -e "${GRAY}     [7] 193.136.128.109 (sigma06)${NC}"
    echo -e "${GRAY}     [8] 193.136.128.110 (sigma07)${NC}"
    echo -e "${GRAY}     [9] 193.136.128.111 (sigma08)${NC}"
    echo -e "${GRAY}     [0] Outro IP (manual)${NC}"
    echo ""
    echo -ne "${CYAN}  Opcao: ${NC}"
    
    read server_choice
    
    case $server_choice in
        1) SERVER_IP="193.136.128.103" ;;
        2) SERVER_IP="193.136.128.104" ;;
        3) SERVER_IP="193.136.128.105" ;;
        4) SERVER_IP="193.136.128.106" ;;
        5) SERVER_IP="193.136.128.107" ;;
        6) SERVER_IP="193.136.128.108" ;;
        7) SERVER_IP="193.136.128.109" ;;
        8) SERVER_IP="193.136.128.110" ;;
        9) SERVER_IP="193.136.128.111" ;;
        0)
            echo ""
            echo -ne "${CYAN}  Digite o IP do servidor: ${NC}"
            read SERVER_IP
            ;;
        *)
            SERVER_IP="193.136.128.108"
            echo ""
            echo -e "${YELLOW}  Opcao invalida. Usando 193.136.128.108 por defeito.${NC}"
            ;;
    esac
    
    echo ""
    echo -ne "${CYAN}  Digite a porta do servidor ES (ENTER para 58061): ${NC}"
    read port_input
    
    if [ -z "$port_input" ]; then
        SERVER_PORT=58061
    else
        SERVER_PORT=$port_input
    fi
    
    echo ""
    echo -ne "${GREEN}  Configuracao: ${NC}"
    echo -e "${YELLOW}$SERVER_IP:$SERVER_PORT${NC}"
    echo ""
    
    sleep 1
}

show_test_menu() {
    clear
    echo ""
    echo -e "${CYAN}  ================================================================${NC}"
    echo -e "${CYAN}  SELECAO DE TESTES - ES SERVER${NC}"
    echo -e "${CYAN}  ================================================================${NC}"
    echo ""
    echo -ne "${GRAY}  Servidor ES: ${NC}"
    echo -e "${YELLOW}$SERVER_IP:$SERVER_PORT${NC}"
    echo -e "${GRAY}  Tejo: tejo.tecnico.ulisboa.pt:59000${NC}"
    echo ""
    echo -e "${BLUE}  ------------------------------------------------------------------${NC}"
    echo -e "${BLUE}  ESCOLHA OS TESTES A EXECUTAR:${NC}"
    echo -e "${BLUE}  ------------------------------------------------------------------${NC}"
    echo ""
    
    echo -e "     ${NC}[1] Testes Basicos (1-8)${NC}"
    echo -e "${GRAY}         -> Validacao das funcionalidades principais${NC}"
    echo ""
    
    echo -e "     ${NC}[2] Testes Adicionais (9-12)${NC}"
    echo -e "${GRAY}         -> Testes complementares do sistema${NC}"
    echo ""
    
    echo -e "     ${NC}[3] Concorrencia - Criacao (21-24)${NC}"
    echo -e "${GRAY}         -> 4 clientes criando 10 eventos em paralelo${NC}"
    echo ""
    
    echo -e "     ${NC}[4] Preparacao Base (25)${NC}"
    echo -e "${GRAY}         -> Popular servidor para testes de download${NC}"
    echo ""
    
    echo -e "     ${NC}[5] Concorrencia - Download (26-29)${NC}"
    echo -e "${GRAY}         -> 4 clientes fazendo download simultaneo${NC}"
    echo ""
    
    echo -e "     ${NC}[6] Sequencia Completa Concorrencia (21-24, 25, 26-29)${NC}"
    echo -e "${GRAY}         -> Todos os testes de concorrencia em ordem${NC}"
    echo ""
    
    echo -ne "     ${NC}[7] TODOS OS TESTES (1-8, 9-12, 21-24, 25, 26-29) ${NC}"
    echo -e "${GREEN}[RECOMENDADO]${NC}"
    echo -e "${GRAY}         -> Bateria completa de testes${NC}"
    echo ""
    
    echo -e "     ${RED}[0] Sair${NC}"
    echo ""
    echo -e "${GRAY}  ------------------------------------------------------------------${NC}"
    echo ""
    echo -ne "${CYAN}  Escolha uma opcao: ${NC}"
    
    read choice
    
    echo ""
    case $choice in
        1)
            SELECTED_TESTS="basic"
            echo -e "${GREEN}  Selecionado: Testes Basicos (1-8)${NC}"
            ;;
        2)
            SELECTED_TESTS="additional"
            echo -e "${GREEN}  Selecionado: Testes Adicionais (9-12)${NC}"
            ;;
        3)
            SELECTED_TESTS="concurrent-create"
            echo -e "${GREEN}  Selecionado: Concorrencia - Criacao (21-24)${NC}"
            ;;
        4)
            SELECTED_TESTS="prepare"
            echo -e "${GREEN}  Selecionado: Preparacao Base (25)${NC}"
            ;;
        5)
            SELECTED_TESTS="concurrent-download"
            echo -e "${GREEN}  Selecionado: Concorrencia - Download (26-29)${NC}"
            ;;
        6)
            SELECTED_TESTS="concurrent-all"
            echo -e "${GREEN}  Selecionado: Sequencia Completa de Concorrencia${NC}"
            ;;
        7)
            SELECTED_TESTS="all"
            echo -e "${GREEN}  Selecionado: TODOS OS TESTES${NC}"
            ;;
        0)
            echo ""
            echo -e "${YELLOW}  Ate breve!${NC}"
            exit 0
            ;;
        *)
            echo ""
            echo -e "${RED}  Opcao invalida!${NC}"
            sleep 2
            show_test_menu
            return
            ;;
    esac
    
    echo ""
    echo -e "${GRAY}  Pressione ENTER para continuar...${NC}"
    read
}

# ==============================================================================
# FUNCOES DE EXECUCAO DE TESTES
# ==============================================================================

run_test() {
    local test_num=$1
    local report_dir="reports"
    
    # Track this test
    EXECUTED_TESTS+=("$test_num")
    
    mkdir -p "$report_dir"
    
    local report_file=$(printf "%s/report%02d.html" "$report_dir" "$test_num")
    
    echo -ne "${YELLOW}  [EXECUTANDO] ${NC}"
    echo "Teste $test_num (enviando para tejo...)"
    
    # Executar teste via nc
    echo "$SERVER_IP $SERVER_PORT $test_num" | nc tejo.tecnico.ulisboa.pt 59000 > "$report_file" 2>&1
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    # Verificar se o report foi criado e tem conteudo
    if [ -f "$report_file" ] && [ $(stat -f%z "$report_file" 2>/dev/null || stat -c%s "$report_file" 2>/dev/null) -gt 100 ]; then
        # Verificar erros de comunicacao
        local content=$(cat "$report_file")
        if grep -qiE "(communication error|connection refused|no route to host|failed to connect|cannot connect)" <<< "$content"; then
            echo ""
            echo -e "${RED}  ================================================================${NC}"
            echo -e "${RED}  ERRO DE COMUNICACAO DETECTADO!${NC}"
            echo -e "${RED}  ================================================================${NC}"
            echo ""
            echo -e "${YELLOW}  O servidor ES nao respondeu. Verifique:${NC}"
            echo ""
            echo -e "${GRAY}    [1] O IP do servidor esta correto? (atualmente: $SERVER_IP)${NC}"
            echo -e "${GRAY}    [2] A porta esta correta? (atualmente: $SERVER_PORT)${NC}"
            echo -e "${GRAY}    [3] O servidor ES esta a correr no sigma?${NC}"
            echo -e "${GRAY}    [4] Existe alguma firewall a bloquear?${NC}"
            echo ""
            echo -e "${CYAN}  Pressione ENTER para voltar ao menu...${NC}"
            read
            return 1
        fi
        
        PASSED_TESTS=$((PASSED_TESTS + 1))
        echo -ne "${GREEN}  [PASSED] ${NC}"
        echo -e "Teste $test_num ${GRAY}-> $report_file${NC}"
    else
        FAILED_TESTS=$((FAILED_TESTS + 1))
        echo -ne "${RED}  [FAILED] ${NC}"
        echo "Teste $test_num"
        echo -e "${RED}     -> Report vazio ou nao criado${NC}"
    fi
    
    sleep 2
}

run_parallel_tests() {
    local test_nums=("$@")
    
    echo -e "${CYAN}  Iniciando ${#test_nums[@]} testes em paralelo...${NC}"
    echo ""
    
    mkdir -p "reports"
    
    # Track these tests
    for test_num in "${test_nums[@]}"; do
        EXECUTED_TESTS+=("$test_num")
    done
    
    for test_num in "${test_nums[@]}"; do
        echo -e "${CYAN}  [LANCANDO] Teste $test_num em nova janela...${NC}"
        
        local report_file=$(printf "reports/report%02d.html" "$test_num")
        
        # Executar em background
        (
            echo "$SERVER_IP $SERVER_PORT $test_num" | nc tejo.tecnico.ulisboa.pt 59000 > "$report_file" 2>&1
        ) &
        
        sleep 0.5
    done
    
    echo ""
    echo -e "${YELLOW}  Aguardando conclusao dos testes paralelos...${NC}"
    echo -e "${GRAY}  (Testes em execucao em background)${NC}"
    echo -e "${GRAY}  (Pressione ENTER para continuar)${NC}"
    echo ""
    read
    
    echo ""
    echo -e "${GREEN}  Continuando... (testes em background finalizados)${NC}"
    echo ""
    echo -e "${GRAY}  ================================================================${NC}"
    echo ""
}

show_db_warning() {
    local message=$1
    
    echo ""
    echo -e "${YELLOW}  ------------------------------------------------------------------${NC}"
    echo -e "${YELLOW}  ATENCAO - LIMPEZA DA BASE DE DADOS${NC}"
    echo -e "${YELLOW}  ------------------------------------------------------------------${NC}"
    echo ""
    echo -e "${YELLOW}$message${NC}"
    echo ""
    echo -e "${CYAN}  Pressione ENTER quando estiver pronto para continuar...${NC}"
    read
}

run_test_groups() {
    case $SELECTED_TESTS in
        basic)
            echo ""
            echo -e "${BLUE}  ------------------------------------------------------------------${NC}"
            echo -e "${BLUE}  >>> FASE 1 - TESTES BASICOS${NC}"
            echo -e "${GRAY}  Validacao das funcionalidades principais (1-8)${NC}"
            echo -e "${BLUE}  ------------------------------------------------------------------${NC}"
            echo ""
            
            for i in {1..8}; do
                if ! run_test $i; then
                    return 1
                fi
            done
            ;;
            
        additional)
            echo ""
            echo -e "${BLUE}  ------------------------------------------------------------------${NC}"
            echo -e "${BLUE}  >>> FASE 1 - TESTES ADICIONAIS${NC}"
            echo -e "${GRAY}  Testes complementares do sistema (9-12)${NC}"
            echo -e "${BLUE}  ------------------------------------------------------------------${NC}"
            echo ""
            
            for i in {9..12}; do
                if ! run_test $i; then
                    return 1
                fi
            done
            ;;
            
        concurrent-create)
            echo ""
            echo -e "${BLUE}  ------------------------------------------------------------------${NC}"
            echo -e "${BLUE}  >>> FASE 1 - CONCORRENCIA - CRIACAO${NC}"
            echo -e "${GRAY}  4 clientes criando 10 eventos em paralelo (21-24)${NC}"
            echo -e "${BLUE}  ------------------------------------------------------------------${NC}"
            echo ""
            
            run_parallel_tests 21 22 23 24
            ;;
            
        prepare)
            echo ""
            echo -e "${BLUE}  ------------------------------------------------------------------${NC}"
            echo -e "${BLUE}  >>> FASE 1 - PREPARACAO BASE${NC}"
            echo -e "${GRAY}  Popular servidor para testes de download (25)${NC}"
            echo -e "${BLUE}  ------------------------------------------------------------------${NC}"
            echo ""
            
            if ! run_test 25; then
                return 1
            fi
            ;;
            
        concurrent-download)
            echo ""
            echo -e "${BLUE}  ------------------------------------------------------------------${NC}"
            echo -e "${BLUE}  >>> FASE 1 - CONCORRENCIA - DOWNLOAD${NC}"
            echo -e "${GRAY}  4 clientes fazendo download simultaneo (26-29)${NC}"
            echo -e "${BLUE}  ------------------------------------------------------------------${NC}"
            echo ""
            
            run_parallel_tests 26 27 28 29
            ;;
            
        concurrent-all)
            echo ""
            echo -e "${BLUE}  ------------------------------------------------------------------${NC}"
            echo -e "${BLUE}  >>> FASE 1 - CONCORRENCIA - CRIACAO${NC}"
            echo -e "${GRAY}  4 clientes criando 10 eventos em paralelo (21-24)${NC}"
            echo -e "${BLUE}  ------------------------------------------------------------------${NC}"
            echo ""
            
            run_parallel_tests 21 22 23 24
            
            show_db_warning "  Antes de executar o teste 25 (preparacao), APAGUE a base\n  de dados do servidor no Tejo!"
            
            echo ""
            echo -e "${BLUE}  ------------------------------------------------------------------${NC}"
            echo -e "${BLUE}  >>> FASE 2 - PREPARACAO BASE${NC}"
            echo -e "${GRAY}  Popular servidor para testes de download (25)${NC}"
            echo -e "${BLUE}  ------------------------------------------------------------------${NC}"
            echo ""
            
            if ! run_test 25; then
                return 1
            fi
            
            echo ""
            echo -e "${YELLOW}  ------------------------------------------------------------------${NC}"
            echo -e "${YELLOW}  ATENCAO - NAO APAGUE A BASE DE DADOS AGORA!${NC}"
            echo -e "${YELLOW}  ------------------------------------------------------------------${NC}"
            echo ""
            echo -e "${GREEN}  O teste 25 criou eventos para os testes de download (26-29).${NC}"
            echo -e "${GREEN}  Nao apague a base de dados antes dos proximos testes!${NC}"
            echo ""
            echo -e "${CYAN}  Pressione ENTER para continuar com os testes de download...${NC}"
            read
            
            echo ""
            echo -e "${BLUE}  ------------------------------------------------------------------${NC}"
            echo -e "${BLUE}  >>> FASE 3 - CONCORRENCIA - DOWNLOAD${NC}"
            echo -e "${GRAY}  4 clientes fazendo download simultaneo (26-29)${NC}"
            echo -e "${BLUE}  ------------------------------------------------------------------${NC}"
            echo ""
            
            run_parallel_tests 26 27 28 29
            ;;
            
        all)
            # Testes 1-8
            echo ""
            echo -e "${BLUE}  ------------------------------------------------------------------${NC}"
            echo -e "${BLUE}  >>> FASE 1 - TESTES BASICOS${NC}"
            echo -e "${GRAY}  Validacao das funcionalidades principais (1-8)${NC}"
            echo -e "${BLUE}  ------------------------------------------------------------------${NC}"
            echo ""
            
            for i in {1..8}; do
                if ! run_test $i; then
                    return 1
                fi
            done
            
            show_db_warning "  Antes de continuar para os proximos testes, APAGUE a base\n  de dados do servidor no Tejo para evitar conflitos!"
            
            # Testes 9-12
            echo ""
            echo -e "${BLUE}  ------------------------------------------------------------------${NC}"
            echo -e "${BLUE}  >>> FASE 2 - TESTES ADICIONAIS${NC}"
            echo -e "${GRAY}  Testes complementares do sistema (9-12)${NC}"
            echo -e "${BLUE}  ------------------------------------------------------------------${NC}"
            echo ""
            
            for i in {9..12}; do
                if ! run_test $i; then
                    return 1
                fi
            done
            
            show_db_warning "  Antes de continuar para os testes de concorrencia, APAGUE\n  a base de dados do servidor no Tejo!"
            
            # Testes 21-24
            echo ""
            echo -e "${BLUE}  ------------------------------------------------------------------${NC}"
            echo -e "${BLUE}  >>> FASE 3 - CONCORRENCIA - CRIACAO${NC}"
            echo -e "${GRAY}  4 clientes criando 10 eventos em paralelo (21-24)${NC}"
            echo -e "${BLUE}  ------------------------------------------------------------------${NC}"
            echo ""
            
            run_parallel_tests 21 22 23 24
            
            show_db_warning "  Antes de executar o teste 25 (preparacao), APAGUE a base\n  de dados do servidor no Tejo!"
            
            # Teste 25
            echo ""
            echo -e "${BLUE}  ------------------------------------------------------------------${NC}"
            echo -e "${BLUE}  >>> FASE 4 - PREPARACAO BASE${NC}"
            echo -e "${GRAY}  Popular servidor para testes de download (25)${NC}"
            echo -e "${BLUE}  ------------------------------------------------------------------${NC}"
            echo ""
            
            if ! run_test 25; then
                return 1
            fi
            
            echo ""
            echo -e "${YELLOW}  ------------------------------------------------------------------${NC}"
            echo -e "${YELLOW}  ATENCAO - NAO APAGUE A BASE DE DADOS AGORA!${NC}"
            echo -e "${YELLOW}  ------------------------------------------------------------------${NC}"
            echo ""
            echo -e "${GREEN}  O teste 25 criou eventos para os testes de download (26-29).${NC}"
            echo -e "${GREEN}  Nao apague a base de dados antes dos proximos testes!${NC}"
            echo ""
            echo -e "${CYAN}  Pressione ENTER para continuar com os testes de download...${NC}"
            read
            
            # Testes 26-29
            echo ""
            echo -e "${BLUE}  ------------------------------------------------------------------${NC}"
            echo -e "${BLUE}  >>> FASE 5 - CONCORRENCIA - DOWNLOAD${NC}"
            echo -e "${GRAY}  4 clientes fazendo download simultaneo (26-29)${NC}"
            echo -e "${BLUE}  ------------------------------------------------------------------${NC}"
            echo ""
            
            run_parallel_tests 26 27 28 29
            ;;
    esac
}

show_report() {
    echo ""
    echo -e "${CYAN}  ================================================================${NC}"
    echo -e "${CYAN}  RELATORIO FINAL${NC}"
    echo -e "${CYAN}  ================================================================${NC}"
    echo ""
    
    echo -e "${BLUE}  ------------------------------------------------------------------${NC}"
    echo -e "${BLUE}  ESTATISTICAS${NC}"
    echo -e "${BLUE}  ------------------------------------------------------------------${NC}"
    
    local success_rate=0
    if [ $TOTAL_TESTS -gt 0 ]; then
        success_rate=$((PASSED_TESTS * 100 / TOTAL_TESTS))
    fi
    
    echo -ne "${GRAY}  Total de Testes Sequenciais: ${NC}"
    echo "$TOTAL_TESTS"
    
    echo -ne "${GRAY}  Passaram:                    ${NC}"
    echo -e "${GREEN}$PASSED_TESTS${NC}"
    
    echo -ne "${GRAY}  Falharam:                    ${NC}"
    echo -e "${RED}$FAILED_TESTS${NC}"
    
    echo -ne "${GRAY}  Taxa de Sucesso:             ${NC}"
    if [ $success_rate -ge 80 ]; then
        echo -e "${GREEN}$success_rate%${NC}"
    elif [ $success_rate -ge 50 ]; then
        echo -e "${YELLOW}$success_rate%${NC}"
    else
        echo -e "${RED}$success_rate%${NC}"
    fi
    
    echo ""
    echo -e "${GRAY}  Reports guardados em: ./reports/${NC}"
    echo -e "${GRAY}  ------------------------------------------------------------------${NC}"
    echo ""
    
    # Analisar reports HTML
    echo -e "${CYAN}  ------------------------------------------------------------------${NC}"
    echo -e "${CYAN}  ANALISE DOS REPORTS HTML${NC}"
    echo -e "${CYAN}  ------------------------------------------------------------------${NC}"
    echo ""
    echo -e "${GRAY}  Analisando reports em busca de erros...${NC}"
    echo ""
    
    local has_html_errors=0
    
    # Only check reports for tests that were executed in this session
    for test_num in "${EXECUTED_TESTS[@]}"; do
        local report_file=$(printf "reports/report%02d.html" "$test_num")
        
        if [ -f "$report_file" ]; then
            local content=$(cat "$report_file")
            local report_name=$(basename "$report_file")
            
            if grep -qE "(test.*failed|all tests failed|some tests failed|erro:|error:|timeout|No user logged in)" <<< "$content"; then
                has_html_errors=1
                echo -ne "    ${RED}[X] ${NC}"
                echo -ne "${YELLOW}$report_name${NC}"
                
                if grep -q "No user logged in" <<< "$content"; then
                    echo -e " ${RED}- No user logged in (login falhou)${NC}"
                elif grep -q "timeout" <<< "$content"; then
                    echo -e " ${RED}- Timeout detectado${NC}"
                else
                    echo -e " ${RED}- Erro detectado${NC}"
                fi
            fi
        fi
    done
    
    if [ $has_html_errors -eq 0 ]; then
        echo -e "${GREEN}  [OK] Nenhum erro detectado nos reports HTML!${NC}"
    fi
    
    echo ""
    echo -e "${GRAY}  ------------------------------------------------------------------${NC}"
    echo ""
    
    echo -e "${GREEN}  ===================================================================================================${NC}"
    echo -e "${GREEN}  Testes Completo! Reports em ./reports/{NC}"
    echo -e "${GREEN}  ===================================================================================================${NC}"
    echo ""
    echo -e "${GRAY}  Pressione ENTER para continuar...${NC}"
    read
}

show_continue_menu() {
    echo ""
    echo -e "${YELLOW}  ------------------------------------------------------------------${NC}"
    echo -e "${YELLOW}  ATENCAO - BASE DE DADOS${NC}"
    echo -e "${YELLOW}  ------------------------------------------------------------------${NC}"
    echo ""
    echo -e "${YELLOW}  IMPORTANTE: Entre execucoes de testes pode ser necessario${NC}"
    echo -e "${YELLOW}  apagar a base de dados do servidor no Tejo para evitar${NC}"
    echo -e "${YELLOW}  conflitos de dados (eventos duplicados, IDs repetidos, etc.)${NC}"
    echo ""
    echo -e "${YELLOW}  Certifique-se de limpar a base de dados se necessario${NC}"
    echo -e "${YELLOW}  antes de executar novos testes!${NC}"
    echo ""
    echo -e "${GRAY}  ------------------------------------------------------------------${NC}"
    echo ""
    echo -e "${CYAN}  Deseja executar mais testes?${NC}"
    echo ""
    echo -e "     ${NC}[1] Sim - voltar ao menu de testes${NC}"
    echo -e "     ${NC}[2] Nao - sair${NC}"
    echo ""
    echo -ne "${CYAN}  Opcao: ${NC}"
    
    read choice
    
    if [ "$choice" = "1" ]; then
        return 0
    else
        return 1
    fi
}

# ==============================================================================
# MAIN
# ==============================================================================

# Verificar se estamos na pasta correta do projeto
check_project_folder

# Menu inicial
show_welcome_menu

# Loop principal
while true; do
    # Reset das estatisticas
    TOTAL_TESTS=0
    PASSED_TESTS=0
    FAILED_TESTS=0
    EXECUTED_TESTS=()
    
    # Menu de selecao de testes
    show_test_menu
    
    # Executar testes
    clear
    echo ""
    echo -e "${CYAN}  ================================================================${NC}"
    echo -e "${CYAN}  ||                                                            ||${NC}"
    echo -e "${CYAN}  ||   ES TESTS - EVENT RESERVATION SERVER TEST SUITE          ||${NC}"
    echo -e "${CYAN}  ||                                                            ||${NC}"
    echo -e "${CYAN}  ================================================================${NC}"
    echo ""
    echo -ne "${GRAY}  Servidor ES: ${NC}"
    echo -e "${YELLOW}$SERVER_IP:$SERVER_PORT${NC}"
    echo -ne "${GRAY}  Tejo Test Server: ${NC}"
    echo -e "${YELLOW}tejo.tecnico.ulisboa.pt:59000${NC}"
    echo -ne "${GRAY}  Data: ${NC}"
    echo -e "${YELLOW}$(date '+%d/%m/%Y %H:%M:%S')${NC}"
    echo ""
    echo -e "${GRAY}  ------------------------------------------------------------------${NC}"
    echo ""
    
    run_test_groups
    
    # Relatorio final
    show_report
    
    # Perguntar se quer continuar
    if ! show_continue_menu; then
        break
    fi
done

echo ""
echo -e "${YELLOW}  Ate breve!${NC}"
echo ""
