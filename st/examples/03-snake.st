PROGRAM snake
  VAR_EXTERNAL
    exec: BOOL;
    inc1: INT;
    mw1: WORD;
    stop1: BOOL;
  END_VAR

  if exec then
    mw1:=SHL(mw1,1);
    if mw1=0 then
      mw1:=1;
    end_if;
  end_if;
  WHILE stop1 DO
    inc1:=inc1-1;
  END_WHILE;
  inc1:=inc1+1;
END_PROGRAM
