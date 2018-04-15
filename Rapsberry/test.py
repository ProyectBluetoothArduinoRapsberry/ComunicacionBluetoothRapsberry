from Tkinter import *

class ScaleDemo( Frame ):
   def __init__( self ):
      Frame.__init__( self )
      self.pack( expand = YES, fill = BOTH )
      self.master.title( "Scale Demo" )
      self.master.geometry( "220x270" )

      self.control = Scale( self, from_ = 0, to = 200,orient = HORIZONTAL, command = self.updateCircle )
      self.control.pack( side = BOTTOM, fill = X )
      self.control.set( 10 )

      self.display = Canvas( self, bg = "white" )
      self.display.pack( expand = YES, fill = BOTH )

   def updateCircle( self, scaleValue ):
      end = int( scaleValue ) + 10
      self.display.delete( "circle" )
      self.display.create_oval( 10, 10, end, end,fill = "red", tags = "circle" )

ScaleDemo().mainloop()
