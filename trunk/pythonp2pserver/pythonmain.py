#This is the main file of the p2p server socket

class BaseClass:
  def TestCall(self, msg):
    print "This is BaseClass TestCall function and it self is ",self
    print "BaseClass->msg:",msg

class DriClass(BaseClass):
  def CallTest(self, msg):
    print "This is DriClass, it self is ", self
    print "CallTest->msg",msg
    TestCall(self, msg)

  def TestCall(self,msg):
    print "This is Drive TestCall function and it self is ",self
    print "Drive->msg:",msg

class ThreadClass:
  def TestCall(self, msg):
    print "This is ThreadClass TestCall function and it self is ",self
    print "ThreadClass->msg:",msg

class ThreadDriClass(ThreadClass,DriClass): pass

if __name__ == '__main__':
  t = ThreadDriClass()
  t1 = ThreadClass()
  t2 = DriClass()

  t.TestCall("Hello t")
  t1.TestCall("Hello t1")
  t2.TestCall("Hello t2")


