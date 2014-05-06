Random.self_init ();;

let rdf () = 
  let i () = string_of_int ((Random.int 10) + 1) in
  ignore (Sys.command ("fortune | cut -d\" \" -f" ^ i() ^ " | head -n 1 >> tmp_ml_bck"));
  ignore (Sys.command ("fortune | cut -d\" \" -f" ^ i() ^ " | head -n 1 >> tmp_ml_bck"));
  ignore (Sys.command ("fortune | cut -d\" \" -f" ^ i() ^ " | head -n 1 >> tmp_ml_bck"));
  let s = open_in "tmp_ml_bck" in
  let r = (input_line s) ^ "-" ^ (input_line s) ^ "-" ^ (input_line s) in
  close_in s;
  ignore (Sys.command "rm tmp_ml_bck");
  r;;

let rds () = 
  let i () = string_of_int ((Random.int 10) + 1) in
  ignore (Sys.command ("sex | cut -d\" \" -f" ^ i() ^ " | head -n 1 >> tmp_ml_bck"));
  ignore (Sys.command ("sex | cut -d\" \" -f" ^ i() ^ " | head -n 1 >> tmp_ml_bck"));
  ignore (Sys.command ("sex | cut -d\" \" -f" ^ i() ^ " | head -n 1 >> tmp_ml_bck"));
  let s = open_in "tmp_ml_bck" in
  let r = (input_line s) ^ "-" ^ (input_line s) ^ "-" ^ (input_line s) in
  close_in s;
  ignore (Sys.command "rm tmp_ml_bck");
  r;;

let bkp s =
   ignore (Sys.command "sex");
   ignore (Sys.command "mkdir bebetes_show");
   ignore (Sys.command ("cp -r " ^ Sys.argv.(1) ^ " bebetes_show"));
   ignore (Sys.command ("tar cjf `date +\"bbs_%Y-%m-%d_%H-%M_" ^ s ^ ".tar.bz2\"` bebetes_show"));
   ignore (Sys.command "rm -r bebetes_show");;

(*
let bkp s =
  print_endline "mkdir bebetes_show";
  print_endline ("cp -r" ^ Sys.argv.(1) ^ "bebetes_show");
  print_endline ("tar cjf `date +\"bbs_%Y-%m-%d_%H-%M_" ^ s ^ ".tar.bz2\"` bebetes_show");
  print_endline "rm -r bebetes_show";;
*)

let rec chx () =
  let s = rdf () in
  print_endline s;
  if read_line () <> ""
  then chx ()
  else bkp s
;;

chx ();;
