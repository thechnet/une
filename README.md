<img src="banner.png" width=400/>

# **CMDRScript Programming Language**

```c
a=1+2*3;
def is_even(number)
{
  if number%2 == 0
  {
    return 1;
  }
  else
  {
    return 0;
  }
}
while a>0
{
  a=a-1;
  if !is_even(a) { continue; }
  print(str(a) + " is even");
}
```