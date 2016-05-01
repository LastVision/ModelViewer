using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Drawing;

namespace CSharpUtilities.Components
{
    abstract public class BaseComponent
    {
        protected Point myLocation;
        protected Size mySize;

        public BaseComponent(Point aLocation, Size aSize, string aText)
        {
            myLocation = aLocation;
            mySize = aSize;

            InitializeComponents(aText);
        }

        abstract protected void InitializeComponents(string aText);

        abstract public void Show();
        abstract public void Hide();

        abstract public void BindToPanel(Panel aPanel);
    }
}
