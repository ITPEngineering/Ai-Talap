(* multivibrator at 0.5Hz
   outputs to OUT1, OUT2 scalars
   enabled by START scalar *)
PROGRAM oscillator
  VAR_EXTERNAL
    out1: BOOL;
    out2: BOOL;
    start: BOOL;
  END_VAR

  VAR
    ton1 : TON;
    ton2 : TON;
  END_VAR

  if start then
  ton1(
       IN := not ton2.q,
       PT := T#1000ms);
  ton2(
       IN := ton1.q,
       PT := T#1000ms);
  end_if;

  out1 := ton1.q;
  out2 := not out1;
END_PROGRAM
