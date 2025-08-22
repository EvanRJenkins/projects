# Compile standard Altera libraries with +acc for full signal visibility
vlog +acc "C:/<insert_quartus_install_path>/quartus/eda/sim_lib/altera_primitives.v"
vlog +acc "C:/<insert_quartus_install_path>/quartus/eda/sim_lib/220model.v"
vlog +acc "C:/<insert_quartus_install_path>/quartus/eda/sim_lib/altera_mf.v"


vlog -sv +acc "C:/<insert_project_directory>/oscilloscopeV1/uart_tx.v"
vlog -sv +acc "C:/<insert_project_directory>/oscilloscopeV1/uart_rx.v"
vlog -sv +acc "C:/<insert_project_directory>/oscilloscopeV1/uart_tb.v"

vsim -t ns work.uart_tb

add wave -recursive *

run -all