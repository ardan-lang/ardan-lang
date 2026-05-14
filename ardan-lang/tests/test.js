const print = console.log;

async function outer(x) {
  print("Before await", x);
  return 7;
}

//print(outer(5));

async function testAsync() {
  print("Inside inner");
  let v = await outer(10);
  print("After await in inner", v);

    let x = await outer(10);
    print("After await2: ", x);

}

testAsync();
print("After calling testAsync");
