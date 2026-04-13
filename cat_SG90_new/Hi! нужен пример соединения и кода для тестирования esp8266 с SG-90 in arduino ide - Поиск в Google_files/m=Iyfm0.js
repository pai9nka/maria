"use strict";loaded_g_0(function(_){var window=this;
_.h("Iyfm0");
var Mtd=class extends _.fz{constructor(a){super("userCommentUpdated");this.detail=a}},Ntd=function(a){var b=!!a.Eh&&(!a.Da||!!a.en);b!==a.za&&(a.za=b,a=a.j,b=new _.TV("submit_button_status_update",{wS:b}),a.dispatchEvent(b))};
_.Lt(class extends _.Mt{constructor(){super(...arguments);this.Eh=null;this.en=this.ba="";this.aa=this.ea=this.da=this.za=this.Da=!1;this.wa=this.j=null;this.ka=[]}oa(a,b,c){this.Da=c;this.Eh!==a?(this.Eh=a,this.ba=b):(this.Eh=null,this.ba="");this.ka.forEach(d=>{var e=new _.TV("issue_chip_status_update",{Eh:this.Eh});d.dispatchEvent(e)});Ntd(this)}Na(a){if(!!a.length!==!!this.en.length)if(a.length>0&&!this.Eh){var b=this.j,c=new _.TV("issue_chip_auto_select",{toggle:!0},!1);b.dispatchEvent(c)}else a.length===
0&&this.Eh&&(b=this.j,c=new _.TV("issue_chip_auto_select",{toggle:!1},!1),b.dispatchEvent(c));this.en=a;Ntd(this)}Ea(a){this.da=a}va(a){this.ea=a}La(a){this.j=a}Fa(a){this.ka.push(a)}Ia(a){this.wa=a}la(a){this.aa=a}reset(){this.Eh=null;this.en=this.ba="";this.aa=!1;this.ka.forEach(a=>{var b=new _.TV("issue_chip_status_update",{Eh:this.Eh});a.dispatchEvent(b)});this.wa.dispatchEvent(new Mtd({en:this.en}));Ntd(this)}},_.nSa);
_.q();
});
// Google Inc.
