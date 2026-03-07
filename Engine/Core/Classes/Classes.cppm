export module Ferrite.Core.Classes;

export import Ferrite.Core.Classes.Component;
export import Ferrite.Core.Classes.Object;
export import Ferrite.Core.Classes.Prefabs;
export import Ferrite.Core.Classes.PrefabManager;

namespace Ferrite::Core {
    export using Component = Classes::Component<Classes::Object>;
    export using Object = Classes::Object;
}
