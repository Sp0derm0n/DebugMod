class MainContainer extends MovieClip
{
	
	function MainContainer()
	{
		super();
	}
	
	function SetY(a_y:Number)
	{
		this._y = a_y;
	}
	
	function GetWidth():Number
	{
		return this._width;
	}
	
	function GetHeight():Number
	{
		return this._height;
	}
	
	function CreateItem(a_linkageName:String, a_newName:String)
	{
		this.attachMovie(a_linkageName, a_newName, this.getNextHighestDepth()) // 0 = depth
	}
}
