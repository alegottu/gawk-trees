function test(x){ if (x==0 && x=="") return 0; else return x }

BEGIN{
	y = "b"
}

{
	if ($1=="yes") x = "test"
}

END{
	print test("mrow")
	if (x==0 && x=="") print 0; else print x
	print test(x)
	if (y==0 && y=="") print 0; else print y
	if (z[0]==0 && z[0]=="") print 0; else print 1
}
