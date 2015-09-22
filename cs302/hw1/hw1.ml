(* HOMEWORK 1 : COMP 302 Fall 2015
   
   PLEASE NOTE:  

   * All code files must be submitted electronically
     BEFORE class on 24 Sep, 2015

  *  The submitted file name must be hw1.ml 

  *  Submitted solutions will be graded according to their correctness and elegance. 
     Please consult the OCaml style guide posted on the course website.

  *  Your program must type-check and run using OCaml of at least OCaml 4.0

  * Remove all "raise NotImplemented" with your solutions
*)

exception NotImplemented
exception Error


(* ------------------------------------------------------------*)
(* QUESTION : Zipping and Unzipping                            *)
(* ------------------------------------------------------------*)

(* zip : 'a list -> 'b list -> ('a * 'b) list = <fun> 
 * This function zip merges two lists of same size into a list of tuples *)
let rec zip l1 l2 = match (l1, l2) with
        | ([],[]) -> []
        | (h1::l1,h2::l2) -> (h1,h2)::(zip l1 l2)
        | _ -> raise Error

	
(* unzip : ('a * 'b) list -> 'a list * 'b list = <fun> 
 * This function unzip extract two lists of same size from a list of tuples *)
let rec unzip l = match l with 
        | [] -> ([],[])
        | (h1,h2)::l -> let (l1,l2) = unzip l in (h1::l1,h2::l2)  

(*

PROVE THE FOLLOWING STATEMENT:

Theorem: unzip (zip l1 l2) = (l1, l2)

Base case: 
let l1=[] and l2=[] 
unzip (zip [] []) 
=> unzip ([])      by zip 
=> ([],[])         by unzip

Induction Hypothesis : 
for some l1 and l2 we have : unzip ( zip l1 l2 ) = unzip (l) = (l1, l2)

Induction step:
let l1'= h1'::l1 and l2'= h2'::l2
unzip (zip l1' l2')
unzip (zip (h1'::l1) (h2'::l2)) by definition of l1' l2'
=>* unzip ((h1',h2')::l) by definition of zip
=>* (h1'::l1,h2'::l2) by definition of unzip and Induction hypothesis.
    (l1',l2') 

So we have shown that unzip (zip l2 l1) returns indeed (l1,l2)
*)



(* ------------------------------------------------------------*)
(* QUESTION : Pocket Calculator                                *)
(* ------------------------------------------------------------*)

type instruction = Plus | Minus | Times | Div | Sin | Cos | Exp | Float of float

type stack = float list

(* instr: instruction -> stack -> stack = <fun>
 * this recursive function instr takes a stack and an instruction 
 * and returns a stack with the modifications made by the instruction *)
let rec instr (i:instruction) (s:stack) = match (i,s) with
        | (Float f,s) ->  f::(s:stack) 
        | (_,[]) -> raise Error 
        | (Sin,h1::s) -> (sin(h1))::(s:stack)
        | (Cos, h1::s) -> (sin(h1))::(s:stack)
        | (_,[x]) -> raise Error 
        | (Plus,h1::h2::s) ->  (h1+.h2)::(s:stack)
        | (Minus,h1::h2::s) -> (h1-.h2)::(s:stack)
        | (Times,h1::h2::s) -> (h1*.h2)::(s:stack)
        | (Div,h1::h2::s) -> (h1/.h2)::(s:stack)
        | (Exp,h1::h2::s) -> (h1**h2)::(s:stack)



(* helper: instruction list -> stack -> float = <fun> 
 * this recursive function helper takes a list of instructions 
 * and a stack. It returns the resulting float of all the inputed values 
 * and instructions *)	
let rec helper is (s:stack) = match (is,s) with 
        | ((Float f)::is,s) -> helper is (f::s) 
        | (_,[]) -> raise Error
        | ((Cos)::is,h1::s) -> helper is ((cos(h1))::s)
        | ((Sin)::is,h1::s) -> helper is ((sin(h1))::s)
        | (((n:instruction)::is),[x]) -> raise Error
        | ((Plus)::is,h1::h2::s) -> helper is ((h1+.h2)::s)
        | ((Minus)::is, h1::h2::s) -> helper is ((h1-.h2)::s)
        | ((Times)::is, h1::h2::s) -> helper is ((h1*.h2)::s)
        | ((Div)::is, h1::h2::s) -> helper is ((h1/.h2)::s)
        | ((Exp)::is, h1::h2::s) -> helper is ((h1**h2)::s)
        | ([],[x]) -> x
        | (_,_) -> raise Error 

(* prog : instruction list -> float = <fun> 
 * this function prog takes a list of instructions and returns a float by
 * calling the helper function and getting its result *)
let prog instrs = helper instrs []

type exp = 
  | PLUS  of exp * exp  (* Plus *)
  | MINUS of exp * exp  (* Minus *)
  | TIMES of exp * exp  (* Times *)
  | DIV   of exp * exp  (* Div *)
  | SIN   of exp        (* Sin *)
  | COS   of exp        (* Cos *)
  | EXP   of exp * exp  (* Exp *)
  | FLOAT of float

(* eval: exp -> float = <fun> 
 * this recursive function exp takes an expression and 
 * returns the resutling float by reading the expression*)
let rec eval e = match e with
 | FLOAT (e1) -> e1
 | PLUS (e1,e2) -> (eval e1) +. (eval e2) 
 | MINUS (e1,e2) -> (eval e1) -. (eval e2)
 | TIMES (e1,e2) -> (eval e1) *. (eval e2) 
 | DIV (e1,e2) -> (eval e1) /. (eval e2) 
 | SIN (e1) -> sin (eval e1) 
 | COS (e1) -> cos (eval e1)
 | EXP (e1,e2) ->  (eval e1)**(eval e2)
 
(* to_instr: exp -> instruction list = <fun> 
 * this recursive to_instr function takes an expression and 
 * returns a list of expressions *)
let rec to_instr e = match e with 
 | FLOAT (e1)-> [Float e1]
 | COS (e1) -> (to_instr e1)@[Cos]
 | SIN (e1) -> (to_instr e1)@[Sin]
 | PLUS (e1,e2)->  (to_instr e1)@(to_instr e2)@[Plus]
 | MINUS (e1,e2) -> (to_instr e1)@(to_instr e2)@[Minus]
 | TIMES (e1,e2) -> (to_instr e1)@(to_instr e2)@[Times]
 | DIV (e1,e2) -> (to_instr e1)@(to_instr e2)@[Div]
 | EXP (e1,e2) -> (to_instr e1)@(to_instr e2)@[Exp]
 
